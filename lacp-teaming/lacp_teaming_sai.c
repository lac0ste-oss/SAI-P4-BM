#ifdef __cplusplus
#include <spdlog/spdlog.h>
extern "C" {
#endif
#include <sai.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <team.h>

const char *test_profile_get_value(_In_ sai_switch_profile_id_t profile_id,
                                   _In_ const char *variable) {
  return NULL;
}
int test_profile_get_next_value(_In_ sai_switch_profile_id_t profile_id,
                                _Out_ const char **variable,
                                _Out_ const char **value) {
  return -1;
}
const service_method_table_t test_services = {test_profile_get_value,
                                              test_profile_get_next_value};
sai_status_t sai_api_initialize(_In_ uint64_t flags,
                                _In_ const service_method_table_t *services);
sai_status_t sai_api_query(_In_ sai_api_t sai_api_id,
                           _Out_ void **api_method_table);
sai_status_t sai_api_uninitialize(void);
#ifdef __cplusplus
}
#endif
/* Enumerate all the K/V pairs in a profile.
Pointer to NULL passed as variable restarts enumeration.
Function returns 0 if next value exists, -1 at the end of the list. */


// team monitor auxillary
static char *get_port_name(struct team_handle *th, uint32_t ifindex) {
  static char ifname[32];

  return team_ifindex2ifname(th, ifindex, ifname, sizeof(ifname));
}

static int die = 0;

static void sigint_handler(int signum) { die = 1; }

static void do_main_loop(struct team_handle *th) {
  fd_set rfds;
  fd_set rfds_tmp;
  int fdmax;
  int ret;
  int tfd;

  FD_ZERO(&rfds);
  tfd = team_get_event_fd(th);
  FD_SET(tfd, &rfds);
  fdmax = tfd + 1;

  while (1) {
    rfds_tmp = rfds;
    ret = select(fdmax, &rfds_tmp, NULL, NULL, NULL);
    if (die) break;
    if (ret == -1) {
      perror("select()");
    }
    if (FD_ISSET(tfd, &rfds_tmp)) team_handle_events(th);
  }
}

static int option_change_handler_func(struct team_handle *th, void *arg,
                                      team_change_type_mask_t type_mask) {
  struct team_option *option;
  // printf("------------------\noption change\n\toption list:\n");
  bool carr_up;
  struct team_port *port;  

  sai_lag_api_t* lag_api;
  sai_api_query(SAI_API_LAG, (void**)&lag_api);

  team_for_each_option(option, th) {
    if (team_is_option_changed(option) & (strcmp("enabled", team_get_option_name(option)) == 0)) {
      team_carrier_get(th, &carr_up);
      if (carr_up) {
          sai_object_id_t lag_id, s_id = 0;
          sai_attribute_t sai_attr;
          lag_api->create_lag(&lag_id, s_id, 0, &sai_attr);
          team_for_each_port(port, th) {
            if (team_is_port_link_up(port)) {
              printf("add lag member %s\n", get_port_name(th, team_get_port_ifindex(port)));
            }
          }
      } else {
        printf("REMOVE LAG\n");
      }
    }
  }
  return 0;
}

static struct team_change_handler option_change_handler = {
    .func = option_change_handler_func, .type_mask = TEAM_OPTION_CHANGE,
};

int main() {
  printf("lacp_app initializing\n");
  sai_hostif_api_t* hostif_api;
  sai_api_initialize(0, &test_services);
  sai_api_query(SAI_API_HOSTIF, (void**)&hostif_api);
  sai_object_id_t switch_id = 0;
  sai_object_id_t port_id[2];
  port_id[0] = 12;
  port_id[1] = 14;  // TODO get this from SAI functions

  // create trap group (currently only 1.)
  sai_object_id_t prio_group;

  sai_attribute_t sai_attr_list[2];
  // sai_attr_list[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_PRIO;
  // sai_attr_list[0].value = 7;
  sai_attr_list[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
  sai_attr_list[0].value.u32 = 0;  // high_queue_id; // high_queue_id is a queue
                                   // element created via QoS SAI API
  sai_attr_list[1].id = SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER;
  sai_attr_list[1].value.oid = 0;  // high_policer_id; //high_policer_id is a
                                   // policer element created via policer SAI
                                   // API
  hostif_api->create_hostif_trap_group(&prio_group, switch_id, 2,
                                       sai_attr_list);

  // Create host interface channel
  sai_object_id_t host_if_id[2];
  sai_attribute_t sai_if_channel_attr[3];
  sai_if_channel_attr[0].id = SAI_HOSTIF_ATTR_TYPE;
  sai_if_channel_attr[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
  sai_if_channel_attr[1].id = SAI_HOSTIF_ATTR_OBJ_ID;
  sai_if_channel_attr[1].value.oid =
      port_id[0];  // port_id is a port element created via port SAI API
  sai_if_channel_attr[2].id = SAI_HOSTIF_ATTR_NAME;
  strcpy(sai_if_channel_attr[2].value.chardata,"port5");
  hostif_api->create_hostif(&host_if_id[0], switch_id, 3, sai_if_channel_attr);
  sai_if_channel_attr[0].id = SAI_HOSTIF_ATTR_TYPE;
  sai_if_channel_attr[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
  sai_if_channel_attr[1].id = SAI_HOSTIF_ATTR_OBJ_ID;
  sai_if_channel_attr[1].value.oid =
      port_id[1];  // port_id is a port element created via port SAI API
  sai_if_channel_attr[2].id = SAI_HOSTIF_ATTR_NAME;
  strcpy(sai_if_channel_attr[2].value.chardata,"port6");
  hostif_api->create_hostif(&host_if_id[1], switch_id, 3, sai_if_channel_attr);

  // Configuring Trap-IDs
  sai_attribute_t sai_trap_attr[3];
  sai_object_id_t host_trap_id[1];
  // configure LACP trap_id
  sai_trap_attr[0].id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
  sai_trap_attr[0].value.oid = prio_group;
  sai_trap_attr[1].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
  sai_trap_attr[1].value.s32 = SAI_PACKET_ACTION_TRAP;
  sai_trap_attr[2].id = SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE;
  sai_trap_attr[2].value.s32 = SAI_HOSTIF_TRAP_TYPE_LACP;
  hostif_api->create_hostif_trap(&host_trap_id[0], switch_id, 3, sai_trap_attr);

  // Configuring Host tables
  sai_object_id_t host_table_entry[1];
  sai_if_channel_attr[0].id = SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE;
  sai_if_channel_attr[0].value.s32 = SAI_HOSTIF_TABLE_ENTRY_TYPE_TRAP_ID;
  sai_if_channel_attr[1].id = SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID;
  sai_if_channel_attr[1].value.oid = host_trap_id[0];
  sai_if_channel_attr[2].id = SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE;
  sai_if_channel_attr[2].value.s32 =
      SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT;
  hostif_api->create_hostif_table_entry(&host_table_entry[0], switch_id, 3,
                                        sai_if_channel_attr);
  


  // waitng for lacp link up - TODO
  int status = system("./teamd_sw.sh");

  printf("lacp_app runing\n");
   
   struct team_handle *th;
  int err;
  char *ifname = "team0";
  uint32_t ifindex;

  th = team_alloc();
  if (!th) {
    fprintf(stderr, "team alloc failed.\n");
    return 1;
  }

  ifindex = team_ifname2ifindex(th, ifname);
  if (!ifindex) {
    fprintf(stderr, "Netdevice %s not found.\n", ifname);
    return 1;
  }

  err = team_init(th, ifindex);
  if (err) {
    fprintf(stderr, "team init failed\n");
    return err;
  }

  err = team_change_handler_register(th, &option_change_handler, NULL);
  if (err) {
    fprintf(stderr, "option change handler register failed\n");
    return err;
  }

  signal(SIGINT, sigint_handler);

  do_main_loop(th);

  team_change_handler_unregister(th, &option_change_handler, NULL);
  // team_change_handler_unregister(th, &port_change_handler, NULL);
  team_free(th);




  // removing configurations
  printf("lacp_app teardown initiated\n");
  hostif_api->remove_hostif_table_entry(host_table_entry[0]);
  hostif_api->remove_hostif_trap(host_trap_id[0]);
  hostif_api->remove_hostif(host_if_id[1]);
  hostif_api->remove_hostif(host_if_id[0]);
  hostif_api->remove_hostif_trap_group(prio_group);
  // spdlog::drop_all();
  sai_api_uninitialize();
  return 0;
}
