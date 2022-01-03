#pragma once

#include "ir/builder/exceptions.h"

static BuildInType *getBuildInTypeFor(LangType *type, SrcLocationRange location) {
  auto typeBuildIn = dynamic_cast<BuildInType*>(type);
  if (!typeBuildIn) {
    throw IRGenException("only buildIn types are currently supported", location);
  }
  return typeBuildIn;
}