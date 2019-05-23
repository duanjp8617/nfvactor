NFVactor
=========

This repository contains the source code of NFVactor. The detailed architecture of NFVactor is decribed in paper: https://ieeexplore.ieee.org/document/8633371. 

The source code includes the runtime (in ./runtime directory) and several scripts (in ./eval/scripts directory) for 4 important demos.

Currently, the supported OS is only Ubuntu 16.04.

Build Instructions
-------------------

* Follow https://grpc.io/docs/quickstart/python.html to install python grpcio and grpcio-tools first.

* Install scapy with `sudo python -m pip install scapy`

* Clone this repository to your home folder `~/` with `git clone https://github.com/duanjp8617/nfvactor.git`. To run the demo, cloning into home folder `~/` is a must.

* `cd ~/nfvactor`

* `./setup.sh`

* In case you encounter build error with grpc, please please modify the following line in grpc's Makefile from : <br />
HOST_LDLIBS_PROTOC += $(addprefix -l, $(LIBS_PROTOC)) <br />
to: <br />
HOST_LDLIBS_PROTOC += -L/usr/local/lib $(addprefix -l, $(LIBS_PROTOC)) <br />
Then use `make clean`, `make -j`, `sudo make install` and `sudo ldconfig` to re-build grpc
Source: https://github.com/grpc/grpc/issues/9549

* `cd ~/nfvactor/runtime` and then `make`. This builds the runtime executable `server_main` in `~/nfvactor/runtime/samples/real_rpc_basic`.

* `cd ~/nfvactor/eval/scripts` and then `python gen_grpc.py`. This generates python bindings for the GRPC functions that we use to control the runtimes.

Setup DPDK Execution Environment
-----------------------------------

* `cd ~/nfvactor/deps/bess/deps/dpdk-16.07/tools`

* `sudo ./dpdk-setup.sh`, then enter `21`, followed by entering `2048`. On a server with 2MB huge page size, this will setup 4GB huge pages.

* `sudo modprobe uio_pci_generic`. This loads up a user-space driver for DPDK-compatible NICs.

* `sudo ./dpdk-devbind.py -b uio_pci_generic xx:xx.x`. This binds DPDK-compatible NIC to use uio_pci_generic driver.

Run Demo
=========

Before running the demo. Please use the following commands to start the BESS daemon.

* `cd ~/nfvactor/eval/scripts/`

* `sudo ./reboot_bess.sh`

Throughput Demo
---------------
* This demo shows the throughput of the runtime, using the following command.

* `cd ~/nfvactor/eval/scripts/`

* `python throughput_demo.py`. This creates one virtual switch and one runtime. The traffic generator keeps generating traffic to the virtual switch, which then forward to the runtime. The throughput of the runtime will be printed when the script finishes executing.

* `python cleanup.py`. This cleans things up.

Flow Migration Demo
-------------------
* This demo shows the performance of flow migration, using the following command.

* `cd ~/nfvactor/eval/scripts/`

* `python migration_demo.py`. This creates one virtual switch (runtime 1), runtime 2 and runtime 3. The traffic generator first sends traffic to runtime 2. Then runtime 2 migrates all of its traffic to runtime 3. The migration completion time will be printed when the script finishes executing.

* `python cleanup.py`. This cleans things up.

Replication Throughput Demo
---------------------------
* This demo shows the throughput performance when runtime replication is enabled, using the following command.

* `cd ~/nfvactor/eval/scripts/`

* `python replication_throughput_demo.py`. This creates one virtual switch (runtime 1), runtime 2 and runtime 3. The traffic generator first sends traffic to runtime 2. Then runtime 2 then replicates all of its traffic to to runtime 3. The replication throughput will be printed when the script finishes executing.

* `python cleanup.py`. This cleans things up.

Replication Recovery Demo
---------------------------
* This demo shows the replication recovery performance when runtime replication is enabled, using the following command.

* `cd ~/nfvactor/eval/scripts/`

* `python replication_recovery.py`. This creates one virtual switch (runtime 1), runtime 2 and runtime 3. The traffic generator first sends traffic to runtime 2. Then runtime 2 then replicates all of its traffic to to runtime 3. Finally, we recover all the flows that is originally processed by runtime 2 on runtime 3. The replication recovery time will be printed when the script finishes executing.

* `python cleanup.py`. This cleans things up.

Note
-----
For multi-server and multi-runtime configuration, please contact the author.
