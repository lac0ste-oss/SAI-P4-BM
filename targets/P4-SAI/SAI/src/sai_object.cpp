#include <sstream>
#include <string>
#include "sai_object.h"
#include "../../../../thrift_src/gen-cpp/bm/standard_types.h"
StandardClient* sai_object::bm_client_ptr;
sai_id_map_t* sai_object::sai_id_map_ptr;
Switch_metadata* sai_object::switch_metadata_ptr;

std::string parse_param(uint64_t param, uint32_t num_of_bytes) {
		return std::string(static_cast<char*>(static_cast<void*>(&param)),num_of_bytes);
	}

BmMatchParam parse_exact_match_param(uint64_t param, uint32_t num_of_bytes) {
	BmMatchParam match_param;
	match_param.type = BmMatchParamType::type::EXACT; 
	BmMatchParamExact match_param_exact;
    match_param_exact.key = parse_param(param, num_of_bytes);
    match_param.__set_exact(match_param_exact);
    return match_param;
}


void sai_object::config_port(Port_obj* port){
  BmAddEntryOptions options;
	BmMatchParams match_params;
	match_params.clear();
  match_params.push_back(parse_exact_match_param(port->hw_port,2));
  BmActionData action_data;
  action_data.clear();
  action_data.push_back(parse_param(port->pvid,1));
  action_data.push_back(parse_param(port->bind_mode,1));
  action_data.push_back(parse_param(port->mtu,4));
  action_data.push_back(parse_param(port->drop_tagged,1));
  action_data.push_back(parse_param(port->drop_untagged,1));
  try{bm_client_ptr->bm_mt_modify_entry(cxt_id,"table_port_configurations",port->handle_port_cfg,"action_set_port_configurations",action_data);}
  catch(int e) {printf("InvalidTableOperation while removing table_port_configurations entry\n");}
	printf("port cfg modified, handle= %d\n",port->handle_port_cfg);
    
}

sai_status_t sai_object::create_port (sai_object_id_t *port_id, sai_object_id_t switch_id,uint32_t attr_count,const sai_attribute_t *attr_list){
	printf("create port\n");
	//printf("sai_id_map addr in create port fn :%d\n",&sai_object::sai_id_map);
	Port_obj *port = new Port_obj(sai_id_map_ptr);
	//printf("switch_metadata_ptr->ports.size = %d\n",switch_metadata_ptr->ports.size());
	switch_metadata_ptr->ports[port->sai_object_id] = port;
	// switch_metadata_ptr->ports.insert(std::make_pair(port->sai_object_id,&port));
	//printf("new port sai_id = %d\n",port->sai_object_id);
	//parsing attributes
	sai_attribute_t attribute;
	for(uint32_t i = 0; i < attr_count; i++) {
     attribute =attr_list[i];
     switch (attribute.id) {
     	case SAI_PORT_ATTR_PORT_VLAN_ID:
     		port->pvid = attribute.value.u16;
     	break;
     	case SAI_PORT_ATTR_BIND_MODE:
     		port->bind_mode = attribute.value.s32;
     	break;
     	case SAI_PORT_ATTR_HW_LANE_LIST:
     		port->hw_port = attribute.value.u32list.list[0];
     	break;
     }
  }
  //printf("pvid %d, bind_mode %d, hw_port %d \n", port->pvid, port->bind_mode, port->hw_port);
  BmAddEntryOptions options;
  BmMatchParams match_params;
  BmActionData action_data;

  action_data.clear();
  match_params.clear();
  match_params.push_back(parse_exact_match_param(port->hw_port,2));
  action_data.push_back(parse_param(0,1));
  action_data.push_back(parse_param(port->sai_object_id,1));
  port->handle_lag_if = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_ingress_lag",match_params, "action_set_lag_l2if",  action_data, options);
  //printf("lag handle= %d, ",port->handle_lag_if);

  action_data.clear();
  match_params.clear();
  match_params.push_back(parse_exact_match_param(port->sai_object_id,1));
  action_data.push_back(parse_param(port->pvid,1));
  action_data.push_back(parse_param(port->bind_mode,1));
  action_data.push_back(parse_param(port->mtu,4));
  action_data.push_back(parse_param(port->drop_tagged,1));
  action_data.push_back(parse_param(port->drop_untagged,1));
  port->handle_port_cfg = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_port_configurations",match_params, "action_set_port_configurations",  action_data, options);
  //printf("port cfg handle= %d\n",port->handle_port_cfg);
	*port_id = port->sai_object_id;
	return SAI_STATUS_SUCCESS;
}

sai_status_t sai_object::remove_port(sai_object_id_t port_id){
	printf("remove_port, port_id = %d\n",port_id);
	Port_obj* port = switch_metadata_ptr->ports[port_id];
	try{
		bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_ingress_lag",port->handle_lag_if);
		bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_port_configurations",port->handle_port_cfg);
//		printf("deleted port bm_entries\n");
	}
	catch(int e){printf("unable to remove port tables entries\n");}
	switch_metadata_ptr->ports.erase(port->sai_object_id);
	sai_id_map_ptr->free_id(port->sai_object_id);
	//printf("ports.size=%d\n",switch_metadata_ptr->ports.size());
	return SAI_STATUS_SUCCESS;
}

sai_status_t sai_object::create_bridge (sai_object_id_t *bridge_id, sai_object_id_t switch_id,uint32_t attr_count,const sai_attribute_t *attr_list){
	printf("create bridge\n");
	Bridge_obj *bridge = new Bridge_obj(sai_id_map_ptr,0);
	switch_metadata_ptr->bridges[bridge->sai_object_id] = bridge;
	//parsing attributes
	sai_attribute_t attribute;
	for(uint32_t i = 0; i < attr_count; i++) {
     attribute =attr_list[i];
     switch (attribute.id) {
     	case SAI_BRIDGE_ATTR_TYPE:
     		bridge->bridge_type = attribute.value.s32;
     	break;
     }
  }
	*bridge_id = bridge->sai_object_id;
	return SAI_STATUS_SUCCESS;
}

sai_status_t sai_object::remove_bridge(sai_object_id_t bridge_id){
	printf("remove_bridge, bridge_id = %d\n",bridge_id);
	Bridge_obj* bridge = switch_metadata_ptr->bridges[bridge_id];
	switch_metadata_ptr->bridges.erase(bridge->sai_object_id);
	sai_id_map_ptr->free_id(bridge->sai_object_id);
	//printf("bridges.size=%d\n",switch_metadata_ptr->bridges.size());
	return SAI_STATUS_SUCCESS;
}



sai_status_t sai_object::create_bridge_port (sai_object_id_t *bridge_port_id, sai_object_id_t switch_id,uint32_t attr_count,const sai_attribute_t *attr_list){
	printf("create bridge_port\n");
	BridgePort_obj *bridge_port = new BridgePort_obj(sai_id_map_ptr);
	printf("created obj\n");
	//printf("switch_metadata_ptr->bridge_ports.size = %d\n",switch_metadata_ptr->bridge_ports.size());
	switch_metadata_ptr->bridge_ports[bridge_port->sai_object_id] = bridge_port;
	printf("added likk in metadata\n");
	sai_attribute_t attribute;
	uint32_t bridge_id;
	for(uint32_t i = 0; i < attr_count; i++) {
      attribute =attr_list[i];
      printf("attr id %d\n",attribute.id);
      switch (attribute.id) {
     	case SAI_BRIDGE_PORT_ATTR_VLAN_ID:
     		printf("br_vlan_id=%d,i=%d\n",attribute.value.s32,i);
     		bridge_port->vlan_id = attribute.value.s32;
     		break;
     	case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
     		 printf("br_port_br_id=%d, i=%d\n",attribute.value.s32,i);
     		bridge_id = attribute.value.s32;
     		break;
     	case SAI_BRIDGE_PORT_ATTR_TYPE:
     		printf("br_port_type=%d,i=%d\n",attribute.value.s32,i);
     		bridge_port->bridge_port_type = attribute.value.s32;
     		break;
     	case SAI_BRIDGE_PORT_ATTR_PORT_ID:
     	printf("br_port_port_id=%d,i=%d\n",attribute.value.s32,i);
     		bridge_port->port_id = attribute.value.s32;
     		break;
     }
  }
  BmAddEntryOptions options;
  BmMatchParams match_params;
  BmActionData action_data;
  int32_t l2_if_type;
  // 1D
  if(bridge_port->bridge_port_type==SAI_BRIDGE_PORT_TYPE_SUB_PORT){ 
  	match_params.push_back(parse_exact_match_param(bridge_port->sai_object_id,1));
  	action_data.push_back(parse_param(bridge_id,2));
  	bridge_port->handle_id_1d = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_bridge_id_1d",match_params, "action_set_bridge_id",  action_data, options);
  	action_data.clear();
  	action_data.push_back(parse_param(bridge_port->vlan_id,2));
  	bridge_port->handle_egress_set_vlan = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_egress_set_vlan",match_params, "action_set_vlan",  action_data, options);
  	l2_if_type=2;
  }
  // 1Q
  else{
  	match_params.clear();
  	match_params.push_back(parse_exact_match_param(bridge_port->vlan_id,2));
  	action_data.clear();
  	action_data.push_back(parse_param(bridge_id,2));

  	bridge_port->handle_id_1q = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_bridge_id_1q",match_params, "action_set_bridge_id",  action_data, options);
	l2_if_type=3;
  }
  // TODO add lag check here
  	//if port_id in self.lags.keys(): # LAG
      // self.cli_client.AddTable('table_egress_br_port_to_if', 'action_forward_set_outIfType', str(br_port), list_to_str([port_id, 1]))
      // bind_mode = self.ports[self.lag_members[self.lags[port_id].lag_members[0]].port_id].bind_mode
    //else: # port

	match_params.clear();
  	match_params.push_back(parse_exact_match_param(bridge_port->sai_object_id,1));
  	action_data.clear();
  	Port_obj* port = switch_metadata_ptr->ports[bridge_port->port_id];
  	action_data.push_back(parse_param(port->hw_port,2));
  	action_data.push_back(parse_param(0,1));
  	bridge_port->handle_egress_br_port_to_if = bm_client_ptr->bm_mt_add_entry(cxt_id,"table_egress_br_port_to_if",match_params, "action_forward_set_outIfType",  action_data, options);
    uint32_t bind_mode = port->bind_mode;
    if (bind_mode == SAI_PORT_BIND_MODE_SUB_PORT){
		match_params.clear();
		match_params.push_back(parse_exact_match_param(bridge_port->port_id,2));
	  	match_params.push_back(parse_exact_match_param(bridge_port->vlan_id,2));
	  	action_data.clear();
	  	action_data.push_back(parse_param(l2_if_type,1));
	  	action_data.push_back(parse_param(bridge_port->sai_object_id,1));
	    bridge_port->handle_subport_ingress_interface_type=bm_client_ptr->bm_mt_add_entry(cxt_id,"table_subport_ingress_interface_type",match_params, "action_set_l2_if_type",  action_data, options);
  	}
    else{
		match_params.clear();
	  	match_params.push_back(parse_exact_match_param(bridge_port->port_id,2));
		action_data.clear();
		action_data.push_back(parse_param(bridge_port->sai_object_id,1));
    	bridge_port->handle_port_ingress_interface_type=bm_client_ptr->bm_mt_add_entry(cxt_id,"table_port_ingress_interface_type",match_params, "action_set_l2_if_type",  action_data, options);
    }
    *bridge_port_id = bridge_port->sai_object_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_object::remove_bridge_port(sai_object_id_t bridge_port_id){
	sai_status_t status = SAI_STATUS_SUCCESS;
	printf("remove_bridge_port, bridge_port_id = %d\n",bridge_port_id);
	BridgePort_obj* bridge_port = switch_metadata_ptr->bridge_ports[bridge_port_id];

	try{
		if (bridge_port->handle_id_1d != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_bridge_id_1d",bridge_port->handle_id_1d);
		}
		if (bridge_port->handle_id_1q != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_bridge_id_1q",bridge_port->handle_id_1q);
		}
		if (bridge_port->handle_egress_set_vlan != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_egress_set_vlan",bridge_port->handle_egress_set_vlan);
		}
		if (bridge_port->handle_egress_br_port_to_if != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_egress_br_port_to_if",bridge_port->handle_egress_br_port_to_if);
		}
		if (bridge_port->handle_subport_ingress_interface_type != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_subport_ingress_interface_type",bridge_port->handle_subport_ingress_interface_type);
		}
		if (bridge_port->handle_port_ingress_interface_type != 999){
			bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_port_ingress_interface_type",bridge_port->handle_port_ingress_interface_type);
		}
		printf("deleted bridge port %d bm_entries\n",bridge_port->sai_object_id);
	}
	catch(int e){
		status = SAI_STATUS_FAILURE;
		printf("unable to remove bridge port tables entries\n");
	}
	switch_metadata_ptr->bridge_ports.erase(bridge_port->sai_object_id);
	sai_id_map_ptr->free_id(bridge_port->sai_object_id);
	//printf("ports.size=%d\n",switch_metadata_ptr->ports.size());
	return status;
}

sai_status_t sai_object::create_fdb_entry(const sai_fdb_entry_t *fdb_entry,uint32_t attr_count,const sai_attribute_t *attr_list){
	sai_status_t status = SAI_STATUS_SUCCESS;
	printf("create fdb entry");
	//parsing attributes
	
	uint32_t entry_type;
	uint32_t bridge_port;
	uint32_t packet_action;
	sai_attribute_t attribute;
	for(uint32_t i = 0; i < attr_count; i++) {
     attribute =attr_list[i];
     switch (attribute.id) {
     	case SAI_FDB_ENTRY_ATTR_TYPE:
     		entry_type = attribute.value.s32;
	     	break;
     	case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
     		bridge_port = attribute.value.oid;
     		break;
     	case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
     		packet_action = attribute.value.s32;
     		break;
     	default:
     		break;
    	}
	}
	//out_if_type = 0 # port_type (not lag or router). TODO: check how to do it with SAI
	if (packet_action == SAI_PACKET_ACTION_FORWARD){
	      if (entry_type == SAI_FDB_ENTRY_TYPE_STATIC){
	   		BmAddEntryOptions options;
	  		BmMatchParams match_params;
	  		BmActionData action_data;
			uint64_t mac_address=0;
			for (int i=0; i<6; ++i){
    			mac_address= (mac_address << 8) | fdb_entry->mac_address[i];
			}
			match_params.push_back(parse_exact_match_param(mac_address,6));

			match_params.push_back(parse_exact_match_param(fdb_entry->bridge_id,2));
			action_data.push_back(parse_param(bridge_port,2));
	   		BmEntryHandle handle;
	   		bm_client_ptr->bm_mt_add_entry(cxt_id,"table_fdb",match_params, "action_set_egress_br_port",  action_data, options);
	      }
  	}
	return status;
}

sai_status_t sai_object::remove_fdb_entry(const sai_fdb_entry_t *fdb_entry){
	sai_status_t status = SAI_STATUS_SUCCESS;
	printf("remove fdb entry");
	BmAddEntryOptions options;
	BmMatchParams match_params;
	BmActionData action_data;
	uint64_t mac_address=0;
	for (int i=0; i<6; ++i){
    	mac_address= (mac_address << 8) | fdb_entry->mac_address[i];
	}
	match_params.push_back(parse_exact_match_param(mac_address,6));
	match_params.push_back(parse_exact_match_param(fdb_entry->bridge_id,2));

	BmMtEntry bm_entry;
	bm_client_ptr->bm_mt_get_entry_from_key(bm_entry,cxt_id,"table_fdb",match_params,options);
	bm_client_ptr->bm_mt_delete_entry(cxt_id,"table_fdb",bm_entry.entry_handle);
	return status;
}