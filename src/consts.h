const int CachelineSize = 64;
enum State {M, S, I};
enum PrTsnType {PrRd, PrWr, Flush};
enum BusTsnType {BusRd, BusRdX};