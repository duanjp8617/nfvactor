import cmd
import info
import time

def print_stats():
    cmd_name = "sudo "+info.HOME_DIR+"/deps/bess/bessctl/bessctl show port"
    output = cmd.cmd_with_output(cmd_name)
    print output

def get_server_port_stats(server_env):
    cmd_name = "sudo "+info.HOME_DIR+"/deps/bess/bessctl/bessctl show port"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    output = cmd.cmd_with_output(final)
    current_time = time.time()*1000

    return {"time_ms":current_time, "port_stats":output}

def get_runtime_port_stat(current_server_port_stats, runtime_info):
    port_name = runtime_info["rt_name"]+"_iport"
    lines = current_server_port_stats["port_stats"].split("\n")

    index = 0
    for line in lines:
        if port_name in line:
            break
        index += 1

    packets = long(lines[index+6].split(":")[1].replace(',', ''))
    dropped = long(lines[index+7].split(":")[1].replace(',', ''))
    total_bytes = long(lines[index+8].split(":")[1].replace(',', ''))

    return {"time_ms":current_server_port_stats["time_ms"], \
    "packets":packets, "dropped":dropped, "bytes":total_bytes}

def get_runtime_cport_stat(current_server_port_stats, runtime_info):
    port_name = runtime_info["rt_name"]+"_cport"
    lines = current_server_port_stats["port_stats"].split("\n")

    index = 0
    for line in lines:
        if port_name in line:
            break
        index += 1

    packets = long(lines[index+2].split(":")[1].replace(',', ''))
    dropped = long(lines[index+3].split(":")[1].replace(',', ''))
    total_bytes = long(lines[index+4].split(":")[1].replace(',', ''))

    return {"time_ms":current_server_port_stats["time_ms"], \
    "packets":packets, "dropped":dropped, "bytes":total_bytes}

def get_runtime_throughput_perf(runtime_port_stat_before, runtime_port_stat_after):
    total_time = runtime_port_stat_after["time_ms"] - runtime_port_stat_before["time_ms"]

    total_processed_packets = float(runtime_port_stat_after["packets"] - runtime_port_stat_before["packets"])

    total_dropped_packets = float(runtime_port_stat_after["dropped"] - runtime_port_stat_before["dropped"])

    total_processed_bytes = float(runtime_port_stat_after["bytes"] - runtime_port_stat_before["bytes"])

    return {"total_time":total_time, \
    "total_processed_packets":total_processed_packets, \
    "total_dropped_packets":total_dropped_packets, \
    "total_processed_bytes":total_processed_bytes, \
    "pkts/s":(total_processed_packets/(total_time/1000.0)), \
    "dropped/s":(total_dropped_packets/(total_time/1000.0)), \
    "bytes/s":(total_processed_bytes/(total_time/1000.0)),
    }

def read_runtime_log(server_env, runtime_info):
    cmd_name = "cat "+info.HOME_DIR+"/eval/scripts/"+runtime_info["rt_name"]+"_log.log"

    final = ""

    if server_env["ip"] != "localhost":
        final = "ssh net@"+server_env["ip"]+" "+"\""+cmd_name+"\""
    else:
        final = cmd_name

    output = cmd.cmd_with_output(final)

    return output

def get_latency(runtime_log):
    lines = runtime_log.split("\n")

    index = 0
    for line in lines:
        if "Latency report" in line:
            break
        index += 1

    if(index == len(lines)):
        return ""
    else:
        return lines[index]

def get_runtime_migration_stat(runtime_log):
    lines = runtime_log.split("\n")

    index = 0
    for line in lines:
        if "Successful migration" in line:
            break
        index += 1

    if index == len(lines):
        return {"succeed":False, \
        "successful_migration":0, \
        "failed_migration":0, \
        "null_migration":0, \
        "migration_time_ms":0}

    successful_migration = long(lines[index].split(":")[-1])
    failed_migration = long(lines[index+1].split(":")[-1])
    null_migration = long(lines[index+2].split(":")[-1])
    migration_time_ms = long(lines[index+4].split(" ")[-1].replace("m","").replace("s","").replace(".",""))

    return {"succeed":True, \
    "successful_migration":successful_migration, \
    "failed_migration":failed_migration, \
    "null_migration":null_migration, \
    "migration_time_ms":migration_time_ms}

def get_runtime_replication_stat(runtime_log):
    lines = runtime_log.split("\n")

    index = 0
    for line in lines:
        if "Successful recovery" in line:
            break
        index += 1

    if index == len(lines):
        return {"succeed":False, \
        "successful_recovery":0, \
        "failed_recovery":0, \
        "recovery_time_ms":0}

    successful_recovery = long(lines[index].split(":")[-1])
    failed_recovery = long(lines[index+1].split(":")[-1])
    recovery_time_ms = long(lines[index+2].split(" ")[-1].replace("m","").replace("s","").replace(".",""))

    return {"succeed":True, \
    "successful_recovery":successful_recovery, \
    "failed_recovery":failed_recovery, \
    "recovery_time_ms":recovery_time_ms}

def main():
    r2_port_stats = get_server_port_stats(info.r2_env)
    r2_rt1_iport_stat_before = get_runtime_port_stat(r2_port_stats, info.r2_rt1)
    r2_rt1_cport_stat_before = get_runtime_cport_stat(r2_port_stats, info.r2_rt1)
    time.sleep(10)
    r2_port_stats = get_server_port_stats(info.r2_env)
    r2_rt1_iport_stat_after = get_runtime_port_stat(r2_port_stats, info.r2_rt1)
    r2_rt1_cport_stat_after = get_runtime_cport_stat(r2_port_stats, info.r2_rt1)

    #r1_rt2_port_stat = get_runtime_port_stat(r1_port_stats, info.r1_rt2)

    #r1_rt3_port_stat = get_runtime_port_stat(r1_port_stats, info.r1_rt3)

    print get_runtime_throughput_perf(r2_rt1_iport_stat_before, r2_rt1_iport_stat_after)
    print "------"
    print get_runtime_throughput_perf(r2_rt1_cport_stat_before, r2_rt1_cport_stat_after)
    #print r1_rt2_port_stat
    #print r1_rt3_port_stat

    #r1_rt1_log =  read_runtime_log(info.r1_env, info.r1_rt1)
    #print (get_runtime_replication_stat(r1_rt1_log))

if __name__ == '__main__':
    main()
