# coding=utf-8

class ServerConfig:
    def __init__(self) -> None:
        self.port = 10073
        self.sample_rate = 16000
        self.checkpoint = './exp/20210204_conformer_exp/final.pt'
        self.yaml_path = './exp/20210204_conformer_exp/train.yaml'
        self.vocab_path = './exp/20210204_conformer_exp/words.txt'
        self.beam_size = 10