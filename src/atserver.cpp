#include "atserver.h"

namespace at {

bool AtServer::addVar(cat_variable* var) {
  // TODO: ensure no duplicates
  vars.push_back(*var);
  return true;
}

bool AtServer::addCmd(cat_command* cmd) {
  cmds.push_back(*cmd);
  return true;
}

bool AtServer::begin() {
  if (cmds.size() == 0) {
    LOG_ERROR("No commands defined");
    return false;
  }
  // cmd_group = {
  cmd_group.cmd = &cmds[0],
  cmd_group.cmd_num = cmds.size(),
  // };
  cmd_desc.push_back(&cmd_group);
  // desc = {
  desc.cmd_group = &cmd_desc[0],
  desc.cmd_group_num = cmd_desc.size(),
  desc.buf = working_buf,
  desc.buf_size = sizeof(working_buf),
  // };
  cat_init(&cat, &desc, &iface, NULL);
  initialized = true;
  return initialized;
}

int AtServer::readSerial() {
  if (!initialized)
    return -1;
  return int(cat_service(&cat));
}

void AtServer::getTerminator(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, terminator, buffer_size);
}

void AtServer::getOk(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, verbose ? vres_ok : res_ok, buffer_size);
}

void AtServer::getError(char* buffer, unsigned short buffer_size) {
  strncpy(buffer, verbose ? vres_err : res_err, buffer_size);
}

void AtServer::send(const char* str, bool ok, bool error) {
  if (strlen(str) > 0)
    serial.write(str);
  if (ok) {
    sendOk();
  } else if (error) {
    sendError();
  } else {
    serial.write(terminator);
  }
}

void AtServer::sendOk() {
  serial.write(verbose ? vres_ok : res_ok);
}

void AtServer::sendError() {
  serial.write(verbose ? vres_err : res_err);
}

}   // namespace at