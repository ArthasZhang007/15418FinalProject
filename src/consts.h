const int CachelineSize = 8 * 8;
enum State {M, S, I};
enum PrTsnType {PrRd, PrWr, Flush};
enum BusTsnType {BusRd, BusRdX};