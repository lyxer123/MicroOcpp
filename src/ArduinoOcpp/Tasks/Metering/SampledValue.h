// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2023
// MIT License

#ifndef SAMPLEDVALUE_H
#define SAMPLEDVALUE_H

#include <ArduinoJson.h>
#include <memory>
#include <functional>

namespace ArduinoOcpp {

template <class T>
class SampledValueDeSerializer {
public:
    static T deserialize(const char *str);
    static bool ready(T& val);
    static std::string serialize(T& val);
    static int32_t toInteger(T& val);
};

template <>
class SampledValueDeSerializer<int32_t> { // example class
public:
    static int32_t deserialize(const char *str);
    static bool ready(int32_t& val) {return true;} //int32_t is always valid
    static std::string serialize(int32_t& val);
    static int32_t toInteger(int32_t& val) {return val;} //no conversion required
};

class SampledValueProperties {
private:
    std::string format;
    std::string measurand;
    std::string phase;
    std::string location;
    std::string unit;

public:
    SampledValueProperties() { }
    SampledValueProperties(const SampledValueProperties& other) :
            format(other.format),
            measurand(other.measurand),
            phase(other.phase),
            location(other.location),
            unit(other.unit) { }
    ~SampledValueProperties() = default;

    void setFormat(const char *format) {this->format = format;}
    const std::string& getFormat() const {return format;}
    void setMeasurand(const char *measurand) {this->measurand = measurand;}
    const std::string& getMeasurand() const {return measurand;}
    void setPhase(const char *phase) {this->phase = phase;}
    const std::string& getPhase() const {return phase;}
    void setLocation(const char *location) {this->location = location;}
    const std::string& getLocation() const {return location;}
    void setUnit(const char *unit) {this->unit = unit;}
    const std::string& getUnit() const {return unit;}
};

enum class ReadingContext {
    InterruptionBegin,
    InterruptionEnd,
    Other,
    SampleClock,
    SamplePeriodic,
    TransactionBegin,
    TransactionEnd,
    Trigger,
    NOT_SET
};

namespace Ocpp16 {
const char *serializeReadingContext(ReadingContext context);
ReadingContext deserializeReadingContext(const char *serialized);
}

class SampledValue {
protected:
    const SampledValueProperties& properties;
    const ReadingContext context;
    virtual std::string serializeValue() = 0;
public:
    SampledValue(const SampledValueProperties& properties, ReadingContext context) : properties(properties), context(context) { }
    SampledValue(const SampledValue& other) : properties(other.properties), context(other.context) { }
    virtual ~SampledValue() = default;

    std::unique_ptr<DynamicJsonDocument> toJson();

    virtual operator bool() = 0;
    virtual int32_t toInteger() = 0;
};

template <class T, class DeSerializer>
class SampledValueConcrete : public SampledValue {
private:
    T value;
public:
    SampledValueConcrete(const SampledValueProperties& properties, ReadingContext context, const T&& value) : SampledValue(properties, context), value(value) { }
    SampledValueConcrete(const SampledValueConcrete& other) : SampledValue(other), value(other.value) { }
    ~SampledValueConcrete() = default;

    operator bool() override {return DeSerializer::ready(value);}

    std::string serializeValue() override {return DeSerializer::serialize(value);}

    int32_t toInteger() override { return DeSerializer::toInteger(value);}
};

class SampledValueSampler {
protected:
    SampledValueProperties properties;
public:
    SampledValueSampler(SampledValueProperties properties) : properties(properties) { }
    virtual ~SampledValueSampler() = default;
    virtual std::unique_ptr<SampledValue> takeValue(ReadingContext context) = 0;
    virtual std::unique_ptr<SampledValue> deserializeValue(JsonObject svJson) = 0;
    const SampledValueProperties& getProperties() {return properties;};
};

template <class T, class DeSerializer>
class SampledValueSamplerConcrete : public SampledValueSampler {
private:
    std::function<T(ReadingContext context)> sampler;
public:
    SampledValueSamplerConcrete(SampledValueProperties properties, std::function<T(ReadingContext)> sampler) : SampledValueSampler(properties), sampler(sampler) { }
    std::unique_ptr<SampledValue> takeValue(ReadingContext context) override {
        return std::unique_ptr<SampledValueConcrete<T, DeSerializer>>(new SampledValueConcrete<T, DeSerializer>(
            properties,
            context,
            sampler(context)));
    }
    std::unique_ptr<SampledValue> deserializeValue(JsonObject svJson) override {
        return std::unique_ptr<SampledValueConcrete<T, DeSerializer>>(new SampledValueConcrete<T, DeSerializer>(
            properties,
            Ocpp16::deserializeReadingContext(svJson["context"] | "NOT_SET"),
            DeSerializer::deserialize(svJson["value"] | "")));
    }
};

} //end namespace ArduinoOcpp

#endif
