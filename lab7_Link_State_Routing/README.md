# Lab 7: Link-State Routing (Dijkstra) Simulation

Simulated link-state (LS) routing where each router process runs Dijkstra's algorithm locally to compute least-cost paths to all other nodes. Routers exchange link-cost updates using UDP and maintain a shared neighbor cost table protected by a mutex.

## Files

- **ls_router.c**: Main router program implementing:
  - Thread 1: receives LS update messages from other routers and updates the shared cost table.
  - Thread 2 (main): reads cost changes from the keyboard every 10 seconds, updates the cost table, and broadcasts the new cost to all other routers via UDP.
  - Thread 3: periodically runs Dijkstra's algorithm over the current graph and prints the least-cost distance from this router to every other router.
- **Makefile**: Builds the `ls_router` binary.
- **routers_sample.txt**: Example router info file for `N = 4` routers, all running on `127.0.0.1` with different ports.
- **costs_sample.txt**: Example `4 x 4` cost matrix for the same topology.

## Build

From the repo root:

```bash
cd lab7_Link_State_Routing
make
```

This creates the `ls_router` executable.

## Sample Topology (N = 4)

Sample router info (`routers_sample.txt`):

```text
router0 127.0.0.1 6000
router1 127.0.0.1 6001
router2 127.0.0.1 6002
router3 127.0.0.1 6003
```

Sample cost table (`costs_sample.txt`):

```text
0 1 3 1000
1 0 1 7
3 1 0 2
1000 7 2 0
```

Here `1000` represents **INFINITE** cost (no direct link). This example topology corresponds to:

- Router 0 is connected to 1 (cost 1) and 2 (cost 3).
- Router 1 is connected to 0 (1), 2 (1), and 3 (7).
- Router 2 is connected to 0 (3), 1 (1), and 3 (2).
- Router 3 is connected to 1 (7) and 2 (2).

## Run

You must run one `ls_router` process **per router**. For the sample with `N = 4`, open **four terminals** (all on the same machine if you use `127.0.0.1`).

In each terminal, from `lab7_Link_State_Routing/`:

```bash
./ls_router <id> <num_routers> <routers_file> <cost_table_file>
```

For the provided samples:

```bash
# Terminal 1
./ls_router 0 4 routers_sample.txt costs_sample.txt

# Terminal 2
./ls_router 1 4 routers_sample.txt costs_sample.txt

# Terminal 3
./ls_router 2 4 routers_sample.txt costs_sample.txt

# Terminal 4
./ls_router 3 4 routers_sample.txt costs_sample.txt
```

## Behavior

- **Thread 1 (receiver)**:
  - Listens on the UDP port specified for this router in `routers_sample.txt`.
  - Receives 3-int packets `<router_id><neighbor_id><new_cost>` (network byte order).
  - Updates `costs[router_id][neighbor_id]` and `costs[neighbor_id][router_id]` and prints the current cost table.

- **Thread 2 (keyboard / sender)**:
  - Runs in `main`.
  - Every **10 seconds**, prompts:
    - `any changes? (neighbor_id new_cost):`
  - After you type `neighbor_id new_cost`, it:
    - Updates the local cost table for the link between `myid` and `neighbor_id`.
    - Broadcasts the change to all other routers using a UDP packet `<myid, neighbor_id, new_cost>`.
  - Performs **2 such changes**, then the process waits 30 seconds and exits.

- **Thread 3 (link-state / Dijkstra)**:
  - Sleeps for a random time between **10 and 20 seconds**.
  - Reads the current cost table (under a mutex).
  - Runs Dijkstra's algorithm with this router as the source and prints:
    - `New least-cost distances from router <myid>:` followed by the distance array.

## Example Test: Make the 1–2 Link Expensive

You can use this example to verify that the link-state algorithm and distance updates are working correctly.

1. **Start all four routers** with the sample files (as above).
2. Go to **router 1's terminal**. When it asks:

   ```text
   any changes? (neighbor_id new_cost):
   ```

3. Type:

   ```text
   2 10
   ```

   This changes link cost **1–2** from `1` → `10` (and all routers will learn it via UDP messages).

4. After a few seconds, watch the `New least-cost distances` lines on each router.

**Expected new distances** (after the update has propagated and each router has rerun Dijkstra):

- **Router 0**: becomes `0 1 3 5`
  - 0→2 now 3 (via direct 0–2, since 0–1–2 is now 1+10=11)
  - 0→3 now 5 (via 0–2–3 = 3+2)
- **Router 1**: becomes `1 0 4 6`
  - 1→2 now 4 (via 1–0–2 = 1+3)
  - 1→3 now 6 (via 1–0–2–3 = 1+3+2)
- **Router 2**: becomes `3 4 0 2`
- **Router 3**: becomes `5 6 2 0`

If you see these changes in the printed distance arrays, then your Dijkstra implementation and cost-table updates are behaving as expected.

## Notes

- All shared access to the `costs` matrix is synchronized using a mutex to avoid race conditions between threads.
- Constants are:
  - `#define N 4`
  - `#define INFINITE 1000`
- For your own topologies, update:
  - `N` in `ls_router.c` (and recompile),
  - The router info file,
  - The cost table file to be `N x N`.

