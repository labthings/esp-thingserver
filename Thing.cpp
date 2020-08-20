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

ThingAction::ThingAction(const char *id_, ThingActionObject *(*generator_fn_)(
                                              DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_) {}

ThingAction::ThingAction(
    const char *id_, JsonObject *input_,
    ThingActionObject *(*generator_fn_)(DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_), input(input_) {}

ThingAction::ThingAction(
    const char *id_, const char *title_, const char *description_,
    const char *type_, JsonObject *input_,
    ThingActionObject *(*generator_fn_)(DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_), title(title_),
      description(description_), type(type_), input(input_) {}

void ThingAction::serialize(JsonObject obj, String deviceId,
                            String resourceType) {
  if (title != "") {
    obj["title"] = title;
  }

  if (description != "") {
    obj["description"] = description;
  }

  if (type != "") {
    obj["@type"] = type;
  }

  if (input != nullptr) {
    JsonObject inputObj = obj.createNestedObject("input");
    for (JsonPair kv : *input) {
      inputObj[kv.key()] = kv.value();
    }
  }

  // 2.11 Action object: A links array (An array of Link objects linking
  // to one or more representations of an Action resource, each with an
  // implied default rel=action.)
  JsonArray inline_links = obj.createNestedArray("links");
  JsonObject inline_links_prop = inline_links.createNestedObject();
  inline_links_prop["href"] = "/actions/" + id;
}

ThingItem::ThingItem(const char *id_, const char *description_,
                     ThingDataType type_, const char *atType_)
    : id(id_), description(description_), type(type_), atType(atType_) {}

void ThingItem::setValue(ThingDataValue newValue) {
    // Set the items value to a ThingDataValue instance
    this->value = newValue;
    this->hasChanged = true;
  }

void ThingItem::setValueArray(ThingDataValue newValues[], int n) {
    // Set the items values to an array of ThingDataValue instances
    this->values = newValues;
    this->hasChanged = true;
    this->_isArray = true;
    this->_arrayLength = n;
  }

void ThingItem::setValue(unsigned int index, ThingDataValue newValue) {
    // Set an element of the items values array to a ThingDataValue instance
    this->values[index] = newValue;
  }

void ThingItem::setValue(const char *s) {
    // Set the items value to a string
    *(this->getValue().string) = s;
    this->hasChanged = true;
  }

void ThingItem::setValue(unsigned int index, const char *s) {
    // Set an element of the items values array to a string
    if (this->isArray() && index < this->arrayLength()) {
      *(this->values[index].string) = s;
      this->hasChanged = true;
    }
  }