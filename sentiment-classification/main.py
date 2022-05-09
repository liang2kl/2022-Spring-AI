import argparse
import sys
import time
from typing import Iterator, Tuple
import torch
import torch.nn as nn
import torch.optim as optim
from torchtext.legacy import data
import gensim
import torchmetrics

from model import CNN, LSTM

# Reference: https://github.com/bentrevett/pytorch-sentiment-analysis

OUTPUT_DIM = 1
DATASET_PATH = "dataset"
RESULT_PATH = "result"
FILTER_SIZES = [1, 2, 3]

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
f1 = torchmetrics.F1Score(threshold=0.5).to(device)

def get_save_path(config, suffix):
    if config.model == "LSTM":
        file_name = f"{config.model}-{config.layers}-{config.bidirectional}-{config.dropout}-" \
        f"{config.hidden_dim}-{config.epoch_num}-{config.batch_size}." + suffix
    elif config.model == "CNN":
        file_name = f"{config.model}-{config.epoch_num}-{config.batch_size}-{config.filter_num}." + suffix
    
    path = RESULT_PATH + "/" + file_name
    return path

def get_metrics(preds, target) -> Tuple[float, float]:
    rounded_preds = torch.round(torch.sigmoid(preds))
    f1_score: torch.Tensor = f1(preds, target.int())
    correct = (rounded_preds == target).float()
    acc = correct.sum() / len(correct)
    return acc.item(), f1_score.item()

def train_model(model: nn.Module, config, iterator, optimizer: optim.Optimizer, criterion):
    epoch_acc = 0
    epoch_f1 = 0
    
    model.train()
    
    for batch in iterator:
        optimizer.zero_grad()

        if config.model == "LSTM":
            predictions = model(batch.text[0].to(device), batch.text[1].to(device))
        elif config.model == "CNN":
            predictions = model(batch.text.to(device)).squeeze(1)
        
        loss = criterion(predictions, batch.label.to(device))
        acc, f1 = get_metrics(predictions, batch.label.to(device))

        loss.backward()
        
        optimizer.step()
        
        epoch_f1 += f1
        epoch_acc += acc
        
    return epoch_acc / len(iterator), epoch_f1 / len(iterator)


def evaluate_model(model: nn.Module, config, iterator, criterion):
    epoch_acc = 0
    epoch_f1 = 0
    
    model.eval()
    
    with torch.no_grad():
    
        for batch in iterator:
            if config.model == "LSTM":
                predictions = model(batch.text[0].to(device), batch.text[1].to(device))
            elif config.model == "CNN":
                predictions = model(batch.text.to(device)).squeeze(1)

            acc, f1 = get_metrics(predictions, batch.label.to(device))

            epoch_acc += acc
            epoch_f1 += f1
        
    return epoch_acc / len(iterator), epoch_f1 / len(iterator)


def load_dataset(config) -> Tuple[nn.Embedding, Iterator, Iterator, Iterator]:
    # Define text and label fields
    text = data.Field(
        include_lengths=(config.model == "LSTM"),
        batch_first=(config.model == "CNN"))
    label = data.LabelField(dtype=torch.float)
    fields = {"sentiment": ("label", label), "text": ("text", text)}

    # Load dataset
    train_data, valid_data, test_data = data.TabularDataset.splits(
        path=DATASET_PATH,
        train="train.json",
        validation="validation.json",
        test="test.json",
        format="json",
        fields=fields
    )

    # Load pre-trained word vectors
    gensim_model = gensim.models.KeyedVectors.load_word2vec_format(
        DATASET_PATH + "/wiki_word2vec_50.bin", binary=True
    )
    weights = torch.FloatTensor(gensim_model.vectors)

    # Build vocabulary
    text.build_vocab(train_data)
    label.build_vocab(train_data)

    # Create embedding layer
    embedding = nn.Embedding.from_pretrained(weights)

    # Split into batches
    train_iterator, valid_iterator, test_iterator = data.BucketIterator.splits(
        (train_data, valid_data, test_data), batch_size = config.batch_size,
        sort_key=lambda x: len(x.text), sort_within_batch=True
    )
    
    return embedding, train_iterator, valid_iterator, test_iterator


def init_model(config) -> Tuple[nn.Module, optim.Optimizer, nn.BCEWithLogitsLoss, Iterator, Iterator, Iterator]:
    # Load dataset
    embedding, train_iterator, valid_iterator, test_iterator = load_dataset(config)
    
    # Create model
    if config.model == "CNN":
        model = CNN(config, embedding, FILTER_SIZES, OUTPUT_DIM)
    elif config.model == "LSTM":
        model = LSTM(config, embedding, OUTPUT_DIM)
    # Adam
    optimizer = optim.Adam(model.parameters())
    # Loss function
    criterion = nn.BCEWithLogitsLoss()
    
    # Set device
    model = model.to(device)
    criterion = criterion.to(device)

    return model, optimizer, criterion, train_iterator, valid_iterator, test_iterator


def train(config):
    rnn, optimizer, criterion, train_iterator, valid_iterator, test_iterator = init_model(config)
    best_acc = -1
    best_epoch = -1

    save_path = get_save_path(config, "pt")

    for e in range(config.epoch_num):
        start_t = time.time()
        train_acc, train_f1 = train_model(rnn, config, train_iterator, optimizer, criterion)
        valid_acc, valid_f1 = evaluate_model(rnn, config, valid_iterator, criterion)
        end_t = time.time()

        if valid_acc > best_acc:
            best_acc = valid_acc
            best_epoch = e + 1
            torch.save(rnn.state_dict(), save_path)
        
        print(f'Epoch: {e+1:02} | Epoch Time: {int(end_t - start_t)}s')
        print(f'  Train F1: {train_f1:.3f} | Train Acc: {train_acc*100:.2f}%')
        print(f'   Val. F1: {valid_f1:.3f} |  Val. Acc: {valid_acc*100:.2f}%')

    rnn.load_state_dict(torch.load(save_path))
    test_acc, test_f1 = evaluate_model(rnn, config, test_iterator, criterion)
    print(f'\nTest F1: {test_f1:.3f} | Test Acc: {test_acc*100:.2f}%')

    return test_acc, test_f1, best_epoch


def main(argv):
    parser = argparse.ArgumentParser(description="Sentiment classification model training.")
    parser.add_argument("model", type=str, choices=["LSTM", "CNN"])
    parser.add_argument("--epoch-num", type=int, default=10, help="total training epoches, universal")
    parser.add_argument("--batch-size", type=int, default=64, help="batch size, universal")
    parser.add_argument("--dropout", type=float, default=0, help="dropout rate, universal")
    parser.add_argument("--layers", type=int, default=1, help="number of layers, LSTM only")
    parser.add_argument("--bidirectional", action="store_true", default=False,
        help="whether using bidrectional model, LSTM only")
    parser.add_argument("--hidden-dim", type=int, default=256, help="dimension of hidden layer, LSTM only")
    parser.add_argument("--filter-num", type=int, default=100, help="number of convolutional filters, CNN only")

    config = parser.parse_args(argv)

    for k, v in config.__dict__.items():
        print(k, ": ", v, sep="")
    
    print("")
    
    torch.backends.cudnn.deterministic = True
    
    start = time.time()
    test_acc, test_f1, best_epoch = train(config)
    end = time.time()

    with open(get_save_path(config, "txt"), "w") as file:
        for k, v in config.__dict__.items():
            file.write(f"{k}: {v}\n")

        file.write(f"\nBest epoch: {best_epoch}, Acc: {test_acc}, F1: {test_f1}," \
            f"Time: {end - start} secs, Avg time: {(end - start) / config.epoch_num} secs")

if __name__ == "__main__":
    main(sys.argv[1:])
