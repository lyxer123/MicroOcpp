// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2023
// MIT License

#ifndef CANCELRESERVATION_H
#define CANCELRESERVATION_H

#include <ArduinoOcpp/Core/OcppMessage.h>

namespace ArduinoOcpp {
namespace Ocpp16 {

class CancelReservation : public OcppMessage {
private:
    bool found = false;
    const char *errorCode = nullptr;
public:
    CancelReservation();

    const char* getOcppOperationType();

    void processReq(JsonObject payload);

    std::unique_ptr<DynamicJsonDocument> createConf();

    const char *getErrorCode() {return errorCode;}
};

} //end namespace Ocpp16
} //end namespace ArduinoOcpp

#endif
