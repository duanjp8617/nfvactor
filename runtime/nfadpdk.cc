#include <cstdio>
#include <string>
#include <cstring>
#include <syslog.h>

#include <rte_config.h>
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>

#include <glog/logging.h>

#include "./bessport/packet.h"

#include "nfadpdk.h"

using std::string;

static void disable_syslog() {
  setlogmask(0x01);
}

static void enable_syslog() {
  setlogmask(0xff);
}

static ssize_t dpdk_log_init_writer(void *, const char *data, size_t len) {
  enable_syslog();
  LOG(INFO) << std::string(data, len);
  disable_syslog();
  return len;
}

static ssize_t dpdk_log_writer(void *, const char *data, size_t len) {
  LOG(INFO) << std::string(data, len);
  return len;
}

static void nfa_init_eal(const char* argv0){
  int ret;
  FILE *org_stdout;

  int rte_argc = 0;
  const char *rte_argv[16];

  char opt_master_lcore[1024];
  char opt_lcore_bitmap[1024];

  sprintf(opt_master_lcore, "%d", RTE_MAX_LCORE - 1);
  sprintf(opt_lcore_bitmap, "%d@%d", RTE_MAX_LCORE - 1, 0);

  rte_argv[rte_argc++] = argv0;
  rte_argv[rte_argc++] = "--master-lcore";
  rte_argv[rte_argc++] = opt_master_lcore;
  rte_argv[rte_argc++] = "--lcore";
  rte_argv[rte_argc++] = opt_lcore_bitmap;
  rte_argv[rte_argc++] = "-n";
  rte_argv[rte_argc++] = "4";
  rte_argv[rte_argc++] = "--proc-type";
  rte_argv[rte_argc++] = "secondary";
  rte_argv[rte_argc] = nullptr;

  cookie_io_functions_t dpdk_log_init_funcs;
  cookie_io_functions_t dpdk_log_funcs;

  std::memset(&dpdk_log_init_funcs, 0, sizeof(dpdk_log_init_funcs));
  std::memset(&dpdk_log_funcs, 0, sizeof(dpdk_log_funcs));

  dpdk_log_init_funcs.write = &dpdk_log_init_writer;
  dpdk_log_funcs.write = &dpdk_log_writer;

  org_stdout = stdout;
  stdout = fopencookie(nullptr, "w", dpdk_log_init_funcs);

  disable_syslog();

  ret = rte_eal_init(rte_argc, const_cast<char **>(rte_argv));

  if (ret < 0) {
    LOG(ERROR) << "rte_eal_init() failed: ret = " << ret;
    exit(EXIT_FAILURE);
  }

  enable_syslog();
  fclose(stdout);
  stdout = org_stdout;

  rte_openlog_stream(fopencookie(nullptr, "w", dpdk_log_funcs));

  LOG(INFO) << "DPDK EAL finishes initialization";
}

void nfa_init_dpdk(const char* argv0){
  nfa_init_eal(argv0);
  int num_of_loaded_mempool = bess::nfa_load_mempool();

  if(num_of_loaded_mempool == 0){
    LOG(ERROR)<<"Fail to load memory pool";
    exit(EXIT_FAILURE);
  }
}
