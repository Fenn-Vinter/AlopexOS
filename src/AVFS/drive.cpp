#include "AlopexOS/AlopexOS_ErrorCodes.hpp"
#include <AlopexOS/AVFS/avfs.hpp>

static_assert(true); // Forces clangd preamble cutoff

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-parameter"

fn AlopexOS::AVFS::drive::BaseAdress_Get() ->  uptr* { return &this->base_address; }
fn AlopexOS::AVFS::drive::BaseAdress_Get(uptr* base_address) ->  uptr*
    { base_address = &this->base_address; return &this->base_address; }
fn AlopexOS::AVFS::drive::BaseAdress_Cpy() ->  uptr  { return this->base_address;  }
fn AlopexOS::AVFS::drive::BaseAdress_Set(const uptr& base_address) -> AlopexOS::errorCode
    { this->base_address = base_address; return AlopexOS::errorCode::Success; }

fn AlopexOS::AVFS::drive::HHDMOffset_Get() ->  u64* { return &this->hhdm_offset; }
fn AlopexOS::AVFS::drive::HHDMOffset_Get(u64* hhdm_offset) ->  u64*
    { hhdm_offset = &this->hhdm_offset; return &this->hhdm_offset; }
fn AlopexOS::AVFS::drive::HHDMOffset_Cpy() ->  u64  { return this->hhdm_offset;  }
fn AlopexOS::AVFS::drive::HHDMOffset_Set(const u64& hhdm_offset) -> AlopexOS::errorCode
    { this->hhdm_offset = hhdm_offset; return AlopexOS::errorCode::Success; }

fn AlopexOS::AVFS::drive::DeviceHandle_Get() ->  u64* { return &this->device_handle; }
fn AlopexOS::AVFS::drive::DeviceHandle_Get(u64* device_handle) ->  u64*
    { device_handle = &this->device_handle; return &this->device_handle; }
fn AlopexOS::AVFS::drive::DeviceHandle_Cpy() ->  u64  { return this->device_handle;  }
fn AlopexOS::AVFS::drive::DeviceHandle_Set(const u64& device_handle) -> AlopexOS::errorCode
    { this->device_handle = device_handle; return AlopexOS::errorCode::Success; }

fn AlopexOS::AVFS::drive::Name_Get() ->  string* { return &this->name; }
fn AlopexOS::AVFS::drive::Name_Get(string* name) ->  string*
    { name = &this->name; return &this->name; }
fn AlopexOS::AVFS::drive::Name_Cpy() ->  string  { return this->name;  }
fn AlopexOS::AVFS::drive::Name_Set(const string& name) -> AlopexOS::errorCode
    { this->name = name; return AlopexOS::errorCode::Success; }

fn AlopexOS::AVFS::drive::Format_Get() ->  string8* { return &this->format; }
fn AlopexOS::AVFS::drive::Format_Get(string8* format) ->  string8*
    { format = &this->format; return &this->format; }
fn AlopexOS::AVFS::drive::Format_Cpy() ->  string8  { return this->format;  }
fn AlopexOS::AVFS::drive::Format_Set(const string8& format) -> AlopexOS::errorCode
    { this->format = format; return AlopexOS::errorCode::Success; }

#pragma clang diagnostic pop
