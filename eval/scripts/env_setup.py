import cmd
import info
from os.path import expanduser
from os import remove

def setup_bess(server_env):
    cmd_name = "sudo "+info.HOME_DIR+"/deps/bess/bessctl/bessctl run file " +info.HOME_DIR+"/eval/scripts/"+server_env["fname"]

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    cmd.cmd(final)

def clean_bess(server_env):
    cmd_name = "sudo "+info.HOME_DIR+"/deps/bess/bessctl/bessctl daemon reset"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    cmd.cmd(final)

def launch_runtime(server_env, runtime_info, service_chain):
    cmd_name = "sudo nohup "+info.HOME_DIR+ \
    "/runtime/samples/real_rpc_basic/server_main "+ \
    "--runtime_id="+str(runtime_info["runtime_id"])+" "+ \
    "--input_port_mac="+runtime_info["input_port_mac"]+" "+ \
    "--output_port_mac="+runtime_info["output_port_mac"]+" "+ \
    "--control_port_mac="+runtime_info["control_port_mac"]+" "+ \
    "--rpc_ip="+runtime_info["rpc_ip"]+" "+ \
    "--rpc_port="+str(runtime_info["rpc_port"])+" "+ \
    "--input_port="+runtime_info["input_port"]+" "+ \
    "--output_port="+runtime_info["output_port"]+" "+ \
    "--control_port="+runtime_info["control_port"]+" "+ \
    "--worker_core="+str(runtime_info["worker_core"])+" "+ \
    "--service_chain="+service_chain+" "+ \
    "> "+info.HOME_DIR+"/eval/scripts/"+runtime_info["log_name"]+" 2>&1 &"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    cmd.cmd(final)

def shutdown_runtime(server_env):
    local_cmd = "sudo kill -9 $(ps -ef | grep server_main | grep -v grep | awk '{print $2}')"

    remote_cmd = "sudo kill -9 \$(ps -ef | grep server_main | grep -v grep | awk '{print \$2}')"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+remote_cmd+"\""
    else:
        final = local_cmd

    cmd.cmd(final)

def remove_runtime_logfile(server_env):
    cmd_name = "sudo rm -f "+info.HOME_DIR+"/eval/scripts/*.log"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    cmd.cmd(final)

# Create temporary files to configure traffic generators:
def create_r1_tmp_file():
    cmd.cmd("cp "+info.HOME_DIR+"/eval/scripts/bess_r1_script "+ \
    info.HOME_DIR+"/eval/scripts/bess_r1_script_tmp")

def remove_r1_tmp_file():
    remove(expanduser("~")+'/nfa-ws/eval/scripts/bess_r1_script_tmp')

# Configure the packet template used by the traffic generator:
def add_pkt(template_file_name, pkt_size):
    template_file = open(expanduser("~")+ \
    '/nfa-ws/eval/scripts/'+template_file_name)

    lines = template_file.readlines()

    tmp_bess_file = open(expanduser("~")+ \
    '/nfa-ws/eval/scripts/bess_r1_script_tmp', "a+")

    tmp_bess_file.write("\n# Define packet type\n")

    for line in lines:
        if 'sizzz' in line:
            new_line = line.replace('sizzz',str(pkt_size))
            tmp_bess_file.write(new_line)
        else:
            tmp_bess_file.write(line)

    tmp_bess_file.write("\n")
    template_file.close()
    tmp_bess_file.close()

# Add a traffic generator for a runtime
def add_variable_flows(traffic_gen_name, runtime_info, pps, flow_rate, flow_duration):
    final_traffic_gen_name = traffic_gen_name+"_"+runtime_info["rt_name"]

    tmp_bess_file = open(expanduser("~")+ \
    '/nfa-ws/eval/scripts/bess_r1_script_tmp', "a+")

    tmp_bess_file.write(final_traffic_gen_name+"::FlowGen(template=pkt_data, ")
    tmp_bess_file.write("pps="+str(pps)+", ")
    tmp_bess_file.write("flow_rate="+str(flow_rate)+", ")
    tmp_bess_file.write("flow_duration="+str(flow_duration)+", ")
    tmp_bess_file.write("arrival='exponential', duration='pareto', quick_rampup=1)\n")

    local_runtime_id = runtime_info["runtime_id"]-10
    core_id = local_runtime_id+8
    tmp_bess_file.write("bess.attach_task(\""+final_traffic_gen_name+"\", 0, wid="+str(core_id)+")")

    tmp_bess_file.write("\n")
    tmp_bess_file.close()

def add_fixed_flows(traffic_gen_name, runtime_info, flow_arrival_interval, flow_pps, flow_duration, concurrent_flows):
    final_traffic_gen_name = traffic_gen_name+"_"+runtime_info["rt_name"]

    tmp_bess_file = open(expanduser("~")+ \
    '/nfa-ws/eval/scripts/bess_r1_script_tmp', "a+")

    tmp_bess_file.write(final_traffic_gen_name+"::MyFlowGen(template=pkt_data, ")
    tmp_bess_file.write("flow_arrival_interval="+str(flow_arrival_interval)+", ")
    tmp_bess_file.write("flow_pps="+str(flow_pps)+", ")
    tmp_bess_file.write("flow_duration="+str(flow_duration)+", ")
    tmp_bess_file.write("concurrent_flows="+str(concurrent_flows)+")\n")

    local_runtime_id = runtime_info["runtime_id"]-10
    core_id = local_runtime_id+8
    tmp_bess_file.write("bess.attach_task(\""+final_traffic_gen_name+"\", 0, wid="+str(core_id)+")")

    tmp_bess_file.write("\n")
    tmp_bess_file.close()

# start the traffic generator
def start_traffic_generator(traffic_gen_name, runtime_info):
    final_traffic_gen_name = traffic_gen_name+"_"+runtime_info["rt_name"]
    runtime_port_name = runtime_info['rt_name']+"_iport_portout"

    cmd_name = "sudo "+info.HOME_DIR+ \
    "/deps/bess/bessctl/bessctl add connection "+final_traffic_gen_name+" "+ \
    runtime_port_name

    cmd.cmd(cmd_name)

def shutdown_traffic_generator(traffic_gen_name, runtime_info):
    final_traffic_gen_name = traffic_gen_name+"_"+runtime_info["rt_name"]
    runtime_port_name = runtime_info['rt_name']+"_iport_portout"

    cmd_name = "sudo "+info.HOME_DIR+ \
    "/deps/bess/bessctl/bessctl delete connection "+final_traffic_gen_name+ \
    " ogate"

    cmd.cmd(cmd_name)

def main():
    #remove_r1_tmp_file()
    #create_r1_tmp_file()
    #add_pkt("simple_tcp.template", 64)
    #add_variable_flows("fg1", info.r1_rt1, pps=10000000, flow_rate = 100000, flow_duration = 10.0)
    start_traffic_generator("fg1", info.r1_rt1)
    #start_traffic_generator("fg2", info.r1_rt1)

if __name__ == '__main__':
    main()
