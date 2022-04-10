const int CachelineSize = 64;
const int NCachelines = 30;
enum State {M, S, I};
enum PrTsnType {PrRd, PrWr, Flush};
enum BusTsnType {BusRd, BusRdX};