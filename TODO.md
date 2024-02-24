
## Current plans
 - As the current multithreading is completly wrong a complete refractoring is being done to feature pvs spliting and after that upgrade to DTS.
 - For now the PV move ordering is done through the TT without any feedback from the engine about current pv due to the parallelism structure. After the pvs/dts implementation the pv/multi-pv interface information will be aviliable.
 - TT is for now fixed size -> the option for uci gui to change the size of tt will be added after the pvs/dts will have been implemented.

## Other plans
 + threefold repetition detection
 + aspiration windows
 + opening and endgame book lookups
 + history moves
 + late move reduction
 + null move pruning
 + selective deepening