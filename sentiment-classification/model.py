import torch
import torch.nn as nn
import torch.nn.functional as F

class LSTM(nn.Module):
    def __init__(self, config, embedding: nn.Embedding, output_dim):
        super().__init__()
        self.config = config
        self.embedding = embedding
        
        self.rnn = nn.LSTM(
            embedding.embedding_dim,
            config.hidden_dim, 
            num_layers=config.layers,
            bidirectional=config.bidirectional,
            dropout=config.dropout
        )
        
        self.dropout = nn.Dropout(config.dropout)
        self.linear = nn.Linear(config.hidden_dim * (2 if config.bidirectional else 1), output_dim)

    def forward(self, text, text_lengths):
        embedded = self.dropout(self.embedding(text))

        output, (hidden, cell) = self.rnn(embedded)
        
        if self.config.bidirectional:
            hidden = self.dropout(torch.cat((hidden[-2,:,:], hidden[-1,:,:]), dim=1))
        else:
            hidden = self.dropout(hidden.squeeze(0))
        
        result = self.linear(hidden)
        return result[-1].squeeze(1) if result.dim() == 3 else result.squeeze(1)


class CNN(nn.Module):
    def __init__(self, config, embedding: nn.Embedding, filter_sizes, output_dim):
        super().__init__()
        
        self.embedding = embedding
        embedding_dim = embedding.embedding_dim
        
        self.convs = nn.ModuleList(
            [nn.Conv2d(in_channels = 1, 
                out_channels = config.filter_num, 
                kernel_size = (fs, embedding_dim)) 
            for fs in filter_sizes])
        
        self.linear = nn.Linear(len(filter_sizes) * config.filter_num, output_dim)
        
        self.dropout = nn.Dropout(config.dropout)
        
    def forward(self, text):
        embedded = self.embedding(text)
        embedded = embedded.unsqueeze(1)
        
        conved = [F.relu(conv(embedded)).squeeze(3) for conv in self.convs]
        
        pooled = [F.max_pool1d(conv, conv.shape[2]).squeeze(2) for conv in conved]
        
        cat = self.dropout(torch.cat(pooled, dim = 1))

        return self.linear(cat)
