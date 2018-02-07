// How modules are connected in nfactor framework.

// The first 3 connections are all triggered by an port_inc

// input_port port_inc -> ec_scheduler -> output_port port_out

// control_port port_inc -> message_passing -> output_port port_out (for replication output)
//                                          -> input_port port_out (for the second step in flow migration protocol)

// shared_ring port_inc -> handle_command

// The next 3 connections are all triggered by the modules

// timers

// handle_command

// message_passing -> control_port port_out (to transmit the control mesages out).
