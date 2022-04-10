const int CachelineSize = 64;
const int NCachelines = 30;
enum State {M, S, I};
enum Transaction {PrRd, PrWr, BusRd, BusRdX, Flush};