#include "Thing.h"

ThingActionObject::ThingActionObject(const char *name_,
                                     DynamicJsonDocument *actionRequest_,
                                     void (*start_fn_)(const JsonVariant &),
                                     void (*cancel_fn_)())
    : start_fn(start_fn_), cancel_fn(cancel_fn_), name(name_),
      actionRequest(actionRequest_),
      timeRequested("1970-01-01T00:00:00+00:00"), status("created") {
  generateId();
  timeRequested = getTimeStampString();
}

#ifndef WITHOUT_WS
void ThingActionObject::setNotifyFunction(
    std::function<void(ThingActionObject *)> notify_fn_) {
  notify_fn = notify_fn_;
}
#endif

void ThingActionObject::generateId() {
  for (uint8_t i = 0; i < 16; ++i) {
    char c = (char)random('0', 'g');

    if (c > '9' && c < 'a') {
      --i;
      continue;
    }

    id += c;
  }
}

void ThingActionObject::serialize(JsonObject obj, String deviceId) {
  JsonObject actionRequestObj = actionRequest->as<JsonObject>();
  obj["input"] = actionRequestObj;

  obj["status"] = status;
  obj["timeRequested"] = timeRequested;

  if (timeCompleted != "") {
    obj["timeCompleted"] = timeCompleted;
  }

  obj["id"] = id;
  obj["title"] = name;

  obj["href"] = "/actions/" + name + "/" + id;
}

void ThingActionObject::setStatus(const char *s) {
  status = s;

#ifndef WITHOUT_WS
  if (notify_fn != nullptr) {
    notify_fn(this);
  }
#endif
}

void ThingActionObject::start() {
ThingActionObject:
  setStatus("pending");

  JsonObject actionRequestObj = actionRequest->as<JsonObject>();
  start_fn(actionRequestObj);

  finish();
}

void ThingActionObject::cancel() {
  if (cancel_fn != nullptr) {
    cancel_fn();
  }
}

void ThingActionObject::finish() {
  timeCompleted = getTimeStampString();
ThingActionObject:
  setStatus("completed");
}