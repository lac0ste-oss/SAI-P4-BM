#include "../inc/sai_adapter.h"

sai_status_t sai_adapter::create_rpf_group(
        _Out_ sai_object_id_t *rpf_group_id,
        _In_ sai_object_id_t switch_id) {
	return SAI_STATUS_NOT_IMPLEMENTED;
}
       
sai_status_t sai_adapter::remove_rpf_group(
	_In_ sai_object_id_t rpf_group_id) {
	return SAI_STATUS_NOT_IMPLEMENTED;
}
       
sai_status_t sai_adapter::set_rpf_group_attribute(
        _In_ sai_object_id_t rpf_group_id,
        _In_ const sai_attribute_t *attr) {
	return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_adapter::get_rpf_group_attribute(
        _In_ sai_object_id_t rpf_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list) {
	return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_adapter::create_rpf_group_member(
        _Out_ sai_object_id_t *rpf_group_member_id,
        _In_ sai_object_id_t switch_id) {
        return SAI_STATUS_NOT_IMPLEMENTED;
}
       
sai_status_t sai_adapter::remove_rpf_group_member(
        _In_ sai_object_id_t rpf_group_member_id) {
        return SAI_STATUS_NOT_IMPLEMENTED;
}
       
sai_status_t sai_adapter::set_rpf_group_member_attribute(
        _In_ sai_object_id_t rpf_group_member_id,
        _In_ const sai_attribute_t *attr) {
        return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_adapter::get_rpf_group_member_attribute(
        _In_ sai_object_id_t rpf_group_member_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list) {
        return SAI_STATUS_NOT_IMPLEMENTED;
}