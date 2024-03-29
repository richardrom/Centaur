/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 19/01/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#include "Protocol.hpp"
#include "fmt/core.h"

#ifndef DONT_INCLUDE_RAPIDJSON
#ifdef __clang__
// clang-format off
CENTAUR_WARN_PUSH()
CENTAUR_WARN_OFF("-Wzero-as-null-pointer-constant")
CENTAUR_WARN_OFF("-Wextra-semi-stmt")
CENTAUR_WARN_OFF("-Wshadow")
CENTAUR_WARN_OFF("-Wambiguous-reversed-operator")
CENTAUR_WARN_OFF("-Wsuggest-override")
CENTAUR_WARN_OFF("-Wsuggest-destructor-override")
CENTAUR_WARN_OFF("-Wreserved-id-macro")
CENTAUR_WARN_OFF("-Wdeprecated-declarations")
// clang-format on
#endif /*__clang__*/
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>
#ifdef __clang__
CENTAUR_WARN_POP()
#endif /*__clang__*/
#endif /*#DONT_INCLUDE_RAPIDJSON*/

auto CENTAUR_PROTOCOL_NAMESPACE::ProtocolJSONGenerator::getJSON() -> std::string
{
    std::string json = R"({"data":{)";
    for (const auto &data : m_types)
    {
        std::visit([&json](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            using U = typename std::remove_pointer_t<T>::field_type;
            json += fmt::format(R"("{}":)", arg->name());

            if constexpr (std::is_integral_v<U>)
            {
                if (std::is_same_v<bool, U>)
                {
                    json += fmt::format("{},", (*arg)() ? "true" : "false");
                }
                else
                    json += fmt::format("{},", (*arg)());
            }
            if constexpr (std::is_floating_point_v<U>)
            {
                json += fmt::format("{:.8f},", (*arg)());
            }
            else if constexpr (std::is_same_v<std::string, U>)
            {
                json += fmt::format(R"("{}",)", (*arg)());
            }
            else if constexpr (std::is_same_v<protocol::json_null_type_t, U>)
            {
                json += fmt::format("null,");
            }
        },
            data);
    }
    json.pop_back();
    json += "}}";
    return json;
}

auto CENTAUR_PROTOCOL_NAMESPACE::ProtocolJSONGenerator::fromJSON(const char *json) -> void
{
    rapidjson::Document jsonDoc;

    if (jsonDoc.Parse(json).HasParseError())
    {
        throw std::runtime_error(fmt::format("{}. At Offset: {}", rapidjson::GetParseError_En(jsonDoc.GetParseError()), jsonDoc.GetErrorOffset()).c_str());
    }

    for (const auto &data : m_types)
    {
        std::visit([&jsonDoc](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            using U = typename std::remove_pointer_t<T>::field_type;

            if constexpr (std::is_integral_v<U>)
            {
                if constexpr (std::is_same_v<bool, U>)
                {
                    (*arg)() = jsonDoc["data"][arg->name().data()].GetBool();
                }
                else if constexpr (std::is_signed_v<U>)
                {
                    (*arg)() = static_cast<U>(jsonDoc["data"][arg->name().data()].GetInt64());
                }
                else if constexpr (std::is_unsigned_v<U>)
                {
                    (*arg)() = static_cast<U>(jsonDoc["data"][arg->name().data()].GetUint64());
                }
            }
            if constexpr (std::is_floating_point_v<U>)
            {
                if constexpr (std::is_same_v<double, U>)
                {
                    (*arg)() = jsonDoc["data"][arg->name().data()].GetDouble();
                }
                else if constexpr (std::is_same_v<long double, U>)
                {
                    (*arg)() = static_cast<long double>(jsonDoc["data"][arg->name().data()].GetDouble());
                }
                else if constexpr (std::is_same_v<float, U>)
                {
                    (*arg)() = jsonDoc["data"][arg->name().data()].GetFloat();
                }
            }
            else if constexpr (std::is_same_v<std::string, U>)
            {
                (*arg)() = std::string { jsonDoc["data"][arg->name().data()].GetString(), jsonDoc["data"][arg->name().data()].GetStringLength() };
            }
            else if constexpr (std::is_same_v<protocol::json_null_type_t, U>)
            {
            }
        },
            data);
    }
}
cen::protocol::ProtocolBase::ProtocolBase(uint32_t type) noexcept :
    protocolType { type } { }

auto cen::protocol::ProtocolBase::type() const -> uint32_t { return protocolType; }

auto CENTAUR_PROTOCOL_NAMESPACE::ProtocolBase::json() -> std::string { return generator.getJSON(); }
auto CENTAUR_PROTOCOL_NAMESPACE::ProtocolBase::fromJson(const char *json) -> void { generator.fromJSON(json); }

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_IncomingConnection::Protocol_IncomingConnection() :
    ProtocolBase { type::Protocol_IncomingConnection }
{
    generator.addField(std::addressof(uuid));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_AcceptConnection::Protocol_AcceptConnection() :
    ProtocolBase { type::Protocol_AcceptConnection }
{
    generator.addField(std::addressof(uuid));
    generator.addField(std::addressof(name));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_AcceptedConnection::Protocol_AcceptedConnection() :
    ProtocolBase { type::Protocol_AcceptedConnection }
{
    generator.addField(std::addressof(uuid));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_AccessPluginMetadata::Protocol_AccessPluginMetadata() :
    ProtocolBase { type::Protocol_AccessPluginMetadata }
{
    generator.addField(std::addressof(uuid));
    generator.addField(std::addressof(pluginUUID));
    generator.addField(std::addressof(responseId));
    generator.addField(std::addressof(request));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_PluginMetadataResponse::Protocol_PluginMetadataResponse() :
    ProtocolBase { type::Protocol_PluginMetadataResponse }
{
    generator.addField(std::addressof(responseId));
    generator.addField(std::addressof(metadata));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_MessageBase::Protocol_MessageBase(uint32_t type) :
    ProtocolBase { type }
{
    generator.addField(std::addressof(uuid));
    generator.addField(std::addressof(responseId));
}

cen::protocol::message::Protocol_Icon::Protocol_Icon() :
    Protocol_MessageBase(type::Protocol_Icon)
{
    generator.addField(std::addressof(iconId));
    generator.addField(std::addressof(data));
    generator.addField(std::addressof(format));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_BalancesResponse::Protocol_BalancesResponse() :
    ProtocolBase { type::Protocol_BalancesResponse }
{
    generator.addField(std::addressof(responseId));
    generator.addField(std::addressof(status));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_BalancesAsset::Protocol_BalancesAsset() :
    Protocol_MessageBase(type::Protocol_BalancesAsset)
{
    generator.addField(std::addressof(total));
    generator.addField(std::addressof(asset));
    generator.addField(std::addressof(assetId));
    generator.addField(std::addressof(assetIcon));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_BalancesAssetItem::Protocol_BalancesAssetItem() :
    Protocol_MessageBase(type::Protocol_BalancesAssetItem)
{
    generator.addField(std::addressof(name));
    generator.addField(std::addressof(value));
    generator.addField(std::addressof(handle));
    generator.addField(std::addressof(subHandle));
    generator.addField(std::addressof(icon));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_BalanceAssetUpdate::Protocol_BalanceAssetUpdate() :
    Protocol_MessageBase(type::Protocol_BalanceAssetUpdate)
{
    generator.addField(std::addressof(assetId));
    generator.addField(std::addressof(value));
}

cen::protocol::message::Protocol_BalanceTotalUpdate::Protocol_BalanceTotalUpdate() :
    Protocol_MessageBase(type::Protocol_BalanceTotalUpdate)
{
    generator.addField(std::addressof(assetId));
    generator.addField(std::addressof(display));
    generator.addField(std::addressof(total));
}

CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_BalanceAssetItemUpdate::Protocol_BalanceAssetItemUpdate() :
    Protocol_MessageBase(type::Protocol_BalanceAssetItemUpdate)
{
    generator.addField(std::addressof(subHandle));
    generator.addField(std::addressof(type));
    generator.addField(std::addressof(value));
}
