#pragma once

#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

#include <WiFiUdp.h>

void beginTimeClient();
String getTimeStampString();

#endif