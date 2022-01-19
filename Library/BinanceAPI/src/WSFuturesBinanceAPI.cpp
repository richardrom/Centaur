////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  WSFuturesBinanceAPI.cpp

/**
 * @file  WSFuturesBinanceAPI.cpp
 * @brief
 *
 * @author Ricardo Romero
 * @date 03/12/2021
 *
 * Copyright (c) 2021 - present, Ricardo Romero
 * All rights reserved.
 *
 */

#include "WSFuturesBinanceAPI.hpp"

#include "Binapi.hpp"
#include <random>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wundefined-func-template"
#pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif /*__clang__*/

#include <fmt/color.h>
#include <fmt/core.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

#define SUBSCRIBE_METHOD(stream)            \
    if (!m_running)                         \
        return { stream };                  \
    else                                    \
    {                                       \
        int id = randNumber();              \
        subscribe(std::string(stream), id); \
        return { id };                      \
    }

#define UNSUBSCRIBE_METHOD(stream)            \
    if (m_running)                            \
    {                                         \
        int id = randNumber(false);           \
        unsubscribe(std::string(stream), id); \
        return id;                            \
    }                                         \
    return -1;

#define JTO_STRING(x, y) \
    std::string { x[y].GetString(), x[y].GetStringLength() }

static std::string symbolToLower(const std::string &symbol)
{
    std::string str = symbol;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

static std::string symbolToUpper(const std::string &symbol)
{
    std::string str = symbol;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

trader::api::ws::WSFuturesBinanceAPI::WSFuturesBinanceAPI()
{
    // Init protocol
    m_protocols[0] = { "wsProtocol", WSFuturesBinanceAPI::eventManager, 0, 65536, 0, nullptr, 0 };
    m_protocols[1] = { nullptr, nullptr, 0, 0, 0, nullptr, 0 };
    // lws_set_log_level(LLL_THREAD | LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO, nullptr);
    lws_set_log_level(0, nullptr);
}

int trader::api::ws::WSFuturesBinanceAPI::randNumber(const bool &subscribe)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1'147'483'648);

    auto &list    = subscribe ? m_subscribeIds : m_unsubscribeIds;
    auto generate = [&distrib, &gen]() {
        return distrib(gen);
    };

    while (true)
    {
        int rdn = generate();
        if (!list.contains(rdn))
        {
            list.insert(rdn);
            return rdn;
        }
    }
}

void trader::api::ws::WSFuturesBinanceAPI::subscribe(const std::string &stream, const int &id)
{
    std::string subscribe = fmt::format(R"({{"method": "SUBSCRIBE","params":["{}"],"id":{}}})", stream, id);
    sendData(subscribe);
}

void trader::api::ws::WSFuturesBinanceAPI::unsubscribe(const std::string &stream, const int &id)
{
    std::string unsubscribe = fmt::format(R"({{"method": "UNSUBSCRIBE","params":["{}"],"id":{}}})", stream, id);
    sendData(unsubscribe);
}

void trader::api::ws::WSFuturesBinanceAPI::run(const std::string &endpoint, const bool &multipleStreams)
{
    if (m_running)
        throw(std::runtime_error("WebSocket already running"));

    m_terminate = false;

    if (m_context != nullptr)
        lws_context_destroy(m_context);

    // Init Context
    lws_context_creation_info info {} /*C++ Initialization just to get rid of clang-tidy and clang warnings of uninitialized variables*/;
    memset(&info, 0, sizeof(info)); // Proper zero initialization of a C struct

    info.port      = CONTEXT_PORT_NO_LISTEN;
    info.protocols = m_protocols;
    info.gid       = -1;
    info.uid       = -1;
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    m_context = lws_create_context(&info);

    lws_client_connect_info ccinfo {};
    memset(&ccinfo, 0, sizeof(lws_client_connect_info));
    ccinfo.context        = m_context;
    ccinfo.address        = "fstream.binance.com";
    ccinfo.port           = 443;
    ccinfo.path           = endpoint.c_str();
    ccinfo.host           = lws_canonical_hostname(m_context);
    ccinfo.origin         = "origin";
    ccinfo.protocol       = m_protocols[0].name;
    ccinfo.userdata       = reinterpret_cast<void *>(this);
    ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

    m_lws                 = lws_client_connect_via_info(&ccinfo);

    if (m_lws == nullptr)
        throw std::runtime_error("failed to connect");

    m_combined = multipleStreams;
    while (!m_terminate)
    {
        lws_service(m_context, 0 /*As documentation denotes it, this value is deprecated and must be zero*/);
    }
    lws_context_destroy(m_context);
    m_context = nullptr;
}

void trader::api::ws::WSFuturesBinanceAPI::terminate()
{

    m_terminate = true;
    lws_callback_on_writable(m_lws);
}

void trader::api::ws::WSFuturesBinanceAPI::subscribe([[maybe_unused]] const bool &result, [[maybe_unused]] const int &id)
{
}

void trader::api::ws::WSFuturesBinanceAPI::unsubscribe([[maybe_unused]] const bool &result, [[maybe_unused]] const int &id)
{
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeMarkPrice(const std::string &symbol, const int &update)
{
    assert(update == 1000 || update == 3000);
    const int sec = [&update]() {
        switch (update)
        {
            case 1000:
                return 1;
            case 3000:
                [[fallthrough]];
            default:
                return 3;
        }
    }();
    std::string stream = fmt::format("{}@markPrice@{}s", symbolToLower(symbol), sec);
    SUBSCRIBE_METHOD(stream)
}
std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeMarkPriceAllMarket(const int &update)
{
    assert(update == 1000 || update == 3000);
    const int sec = [&update]() {
        switch (update)
        {
            case 1000:
                return 1;
            case 3000:
                [[fallthrough]];
            default:
                return 3;
        }
    }();
    std::string stream = fmt::format("!markPrice@{}s", sec);
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeAggregateTrade(const std::string &symbol)
{
    std::string stream = fmt::format("{}@aggTrade", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeKline(const std::string &symbol, const BinanceTimeIntervals &interval)
{
    std::string stream = fmt::format("{}@kline_{}", symbolToLower(symbol), trader::api::BinanceAPI::fromIntervalToString(interval));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeContinuousContractKline(const std::string &symbol, const ContractType &ctype, const BinanceTimeIntervals &interval)
{
    const std::string cotc = [&ctype]() -> std::string {
        switch (ctype)
        {
            case ContractType::none:
                [[fallthrough]];
            case ContractType::PERPETUAL:
                return { "perpetual" };
            case ContractType::CURRENT_QUARTER:
                return { "current_quarter" };
            case ContractType::NEXT_QUARTER:
                return { "next_quarter" };
        }
    }();

    std::string stream = fmt::format("{}_{}@continuousKline_{}", symbolToLower(symbol), cotc, BinanceAPI::fromIntervalToString(interval));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeIndividualMiniTicker(const std::string &symbol)
{
    std::string stream = fmt::format("{}@miniTicker", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

int trader::api::ws::WSFuturesBinanceAPI::unsubscribeIndividualMiniTicker(const std::string &symbol)
{
    std::string stream = fmt::format("{}@miniTicker", symbolToLower(symbol));
    UNSUBSCRIBE_METHOD(stream);
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeAllMarketMiniTickers() {
    SUBSCRIBE_METHOD("!miniTicker@arr")
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeIndividualSymbolTicker(const std::string &symbol)
{
    std::string stream = fmt::format("{}@ticker", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeAllMarketTickers() {
    SUBSCRIBE_METHOD("!ticker@arr")
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeIndividualSymbolBookTickerStreams(const std::string &symbol)
{
    std::string stream = fmt::format("{}@bookTicker", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeAllBookTickerStream() {
    SUBSCRIBE_METHOD("!bookTicker")
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeLiquidationOrder(const std::string &symbol)
{
    std::string stream = fmt::format("{}@forceOrder", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeAllMarketLiquidationOrderStream() {
    SUBSCRIBE_METHOD("!forceOrder@arr")
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribePartialBookDepth(const std::string &symbol, const int &levels, const int &update)
{
    assert(levels == 5 || levels == 10 || levels == 20);
    assert(update == 250 || update == 500 || update == 100);
    std::string stream = fmt::format("{}@depth{}@{}ms", symbolToLower(symbol), levels, update);
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeDiffBookDepth(const std::string &symbol, const int &update)
{
    assert(update == 250 || update == 500 || update == 100);
    std::string stream = fmt::format("{}@depth@{}ms", symbolToLower(symbol), update);
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeBLVTInfo(const std::string &tokenName)
{
    std::string stream = fmt::format("{}@tokenNav", symbolToUpper(tokenName));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeBLVTNavKline(const std::string &tokenName, const BinanceTimeIntervals &interval)
{
    std::string stream = fmt::format("{}@nav_Kline_{}", symbolToUpper(tokenName), BinanceAPI::fromIntervalToString(interval));
    SUBSCRIBE_METHOD(stream)
}

std::variant<std::string, int> trader::api::ws::WSFuturesBinanceAPI::subscribeCompositeIndexSymbolInformation(const std::string &symbol)
{
    std::string stream = fmt::format("{}@compositeIndex", symbolToLower(symbol));
    SUBSCRIBE_METHOD(stream)
}

void trader::api::ws::WSFuturesBinanceAPI::sendData(const std::string &msg)
{
    char *str = new char[msg.size() + LWS_PRE + 1];
    memset(str, 0, msg.size() + LWS_PRE);
    strcpy(str + LWS_PRE, msg.c_str());
    lws_write(m_lws, reinterpret_cast<unsigned char *>(str) + LWS_PRE, msg.size(), LWS_WRITE_TEXT);
    // All previous calls are C functions and do not throw, so we can be assured that delete is going to be called
    delete[] str;
}

void trader::api::ws::WSFuturesBinanceAPI::receivedData(const std::string &msg)
{
    rapidjson::Document jsonDoc;
    jsonDoc.Parse(msg.c_str());
    if (jsonDoc.HasParseError())
    {
        fmt::print(fmt::fg(fmt::color::red), "STREAM ERROR: ");
        fmt::print("JSON data is corrupted\n");
        return;
    }

    try
    {
        // Test for (un)subscription messages
        if (jsonDoc.FindMember("result") != jsonDoc.MemberEnd())
        {

            bool result = jsonDoc["result"].IsNull();

            int id      = jsonDoc["id"].GetInt();
            // Find the Id
            auto siid = m_subscribeIds.find(id);
            if (siid != m_subscribeIds.end())
            {
                m_subscribeIds.erase(siid);
                subscribe(result, id);
            }
            else
            {
                auto uiid = m_unsubscribeIds.find(id);
                if (uiid != m_unsubscribeIds.end())
                {
                    unsubscribe(result, id);
                    m_unsubscribeIds.erase(uiid);
                }
            }

            return;
        }

        const rapidjson::Value &value = m_combined ? (jsonDoc.FindMember("data") != jsonDoc.MemberEnd() ? jsonDoc["data"].GetObject() : jsonDoc.GetObject()) : jsonDoc.GetObject();

        std::string type              = JTO_STRING(value, "e");

        if (!value.IsArray())
        {
            if (type == "aggTrade")
            {
                StreamAggregateTrade sat {
                    .aggregateTradeId = value["a"].GetInt64(),
                    .price            = std::stod(JTO_STRING(value, "p")),
                    .qty              = std::stod(JTO_STRING(value, "q")),
                    .firstTradeId     = value["f"].GetInt64(),
                    .lastTradeId      = value["l"].GetInt64(),
                    .tradeTime        = value["T"].GetUint64(),
                    .isMaker          = value["m"].GetBool()
                };
                aggregateTradeStream(JTO_STRING(value, "s"), value["E"].GetUint64(), sat);
            }
            else if (type == "markPriceUpdate")
            {

                StreamMarkPrice smp {
                    .markPrice        = std::stod(JTO_STRING(value, "p")),
                    .indexPrice       = std::stod(JTO_STRING(value, "i")),
                    .estimSettlePrice = std::stod(JTO_STRING(value, "P")),
                    .fundingRate      = std::stod(JTO_STRING(value, "r")),
                    .nextFundingTime  = value["T"].GetUint64(),
                };
                markPriceStream(JTO_STRING(value, "s"), value["E"].GetUint64(), smp);
            }
            else if (type == "kline")
            {
                const auto &cd           = value["k"].GetObject();
                BinanceTimeIntervals bti = BinanceTimeIntervals::i1M;
                std::string interval     = JTO_STRING(cd, "i");
                if (interval == "1m")
                    bti = BinanceTimeIntervals::i1m;
                else if (interval == "3m")
                    bti = BinanceTimeIntervals::i3m;
                else if (interval == "5m")
                    bti = BinanceTimeIntervals::i5m;
                else if (interval == "15m")
                    bti = BinanceTimeIntervals::i15m;
                else if (interval == "30m")
                    bti = BinanceTimeIntervals::i30m;
                else if (interval == "1h")
                    bti = BinanceTimeIntervals::i1h;
                else if (interval == "2h")
                    bti = BinanceTimeIntervals::i2h;
                else if (interval == "4h")
                    bti = BinanceTimeIntervals::i4h;
                else if (interval == "6h")
                    bti = BinanceTimeIntervals::i6h;
                else if (interval == "8h")
                    bti = BinanceTimeIntervals::i8h;
                else if (interval == "12h")
                    bti = BinanceTimeIntervals::i12h;
                else if (interval == "1d")
                    bti = BinanceTimeIntervals::i1d;
                else if (interval == "3d")
                    bti = BinanceTimeIntervals::i3d;
                else if (interval == "1w")
                    bti = BinanceTimeIntervals::i1w;
                else if (interval == "1M")
                    bti = BinanceTimeIntervals::i1M;

                Candlestick cs {
                    .openTime              = cd["t"].GetUint64(),
                    .open                  = std::stod(JTO_STRING(cd, "o")),
                    .high                  = std::stod(JTO_STRING(cd, "h")),
                    .low                   = std::stod(JTO_STRING(cd, "l")),
                    .close                 = std::stod(JTO_STRING(cd, "c")),
                    .volume                = std::stod(JTO_STRING(cd, "v")),
                    .closeTime             = cd["T"].GetUint64(),
                    .quoteAssetVolume      = std::stod(JTO_STRING(cd, "q")),
                    .baseAssetVolume       = std::stod(JTO_STRING(cd, "v")),
                    .numberOfTrades        = cd["n"].GetUint64(),
                    .takerBaseAssetVolume  = std::stod(JTO_STRING(cd, "V")),
                    .takerQuoteAssetVolume = std::stod(JTO_STRING(cd, "Q")),
                    .isClosed              = cd["x"].GetBool(), /// Stream field otherwise always true
                    .firstTradeId          = cd["f"].GetInt64(),
                    .lastTradeId           = cd["L"].GetInt64()
                };
                kline(JTO_STRING(value, "s"), value["E"].GetUint64(), bti, cs);
            }
            else if (type == "continuous_kline")
            {
                const auto &cd           = value["k"].GetObject();
                BinanceTimeIntervals bti = BinanceTimeIntervals::i1M;
                std::string interval     = JTO_STRING(cd, "i");
                if (interval == "1m")
                    bti = BinanceTimeIntervals::i1m;
                else if (interval == "3m")
                    bti = BinanceTimeIntervals::i3m;
                else if (interval == "5m")
                    bti = BinanceTimeIntervals::i5m;
                else if (interval == "15m")
                    bti = BinanceTimeIntervals::i15m;
                else if (interval == "30m")
                    bti = BinanceTimeIntervals::i30m;
                else if (interval == "1h")
                    bti = BinanceTimeIntervals::i1h;
                else if (interval == "2h")
                    bti = BinanceTimeIntervals::i2h;
                else if (interval == "4h")
                    bti = BinanceTimeIntervals::i4h;
                else if (interval == "6h")
                    bti = BinanceTimeIntervals::i6h;
                else if (interval == "8h")
                    bti = BinanceTimeIntervals::i8h;
                else if (interval == "12h")
                    bti = BinanceTimeIntervals::i12h;
                else if (interval == "1d")
                    bti = BinanceTimeIntervals::i1d;
                else if (interval == "3d")
                    bti = BinanceTimeIntervals::i3d;
                else if (interval == "1w")
                    bti = BinanceTimeIntervals::i1w;
                else if (interval == "1M")
                    bti = BinanceTimeIntervals::i1M;

                ContractType ct = ContractType::none;
                std::string cts = JTO_STRING(value, "ct");

                if (cts == "PERPETUAL")
                    ct = ContractType::PERPETUAL;
                else if (cts == "CURRENT_QUARTER")
                    ct = ContractType::CURRENT_QUARTER;
                else if (cts == "NEXT_QUARTER")
                    ct = ContractType::NEXT_QUARTER;

                Candlestick cs {
                    .openTime              = cd["t"].GetUint64(),
                    .open                  = std::stod(JTO_STRING(cd, "o")),
                    .high                  = std::stod(JTO_STRING(cd, "h")),
                    .low                   = std::stod(JTO_STRING(cd, "l")),
                    .close                 = std::stod(JTO_STRING(cd, "c")),
                    .volume                = std::stod(JTO_STRING(cd, "v")),
                    .closeTime             = cd["T"].GetUint64(),
                    .quoteAssetVolume      = std::stod(JTO_STRING(cd, "q")),
                    .baseAssetVolume       = std::stod(JTO_STRING(cd, "v")),
                    .numberOfTrades        = cd["n"].GetUint64(),
                    .takerBaseAssetVolume  = std::stod(JTO_STRING(cd, "V")),
                    .takerQuoteAssetVolume = std::stod(JTO_STRING(cd, "Q")),
                    .isClosed              = cd["x"].GetBool(), /// Stream field otherwise always true
                    .firstTradeId          = cd["f"].GetInt64(),
                    .lastTradeId           = cd["L"].GetInt64()
                };
                continuousKline(JTO_STRING(value, "ps"), value["E"].GetUint64(), ct, bti, cs);
            }

            else if (type == "24hrMiniTicker")
            {
                StreamIndividualSymbolMiniTicker smmt {
                    .closePrice                  = std::stod(JTO_STRING(value, "c")),
                    .openPrice                   = std::stod(JTO_STRING(value, "o")),
                    .highPrice                   = std::stod(JTO_STRING(value, "h")),
                    .lowPrice                    = std::stod(JTO_STRING(value, "l")),
                    .totalTradedBaseAssetVolume  = std::stod(JTO_STRING(value, "v")),
                    .totalTradedQuoteAssetVolume = std::stod(JTO_STRING(value, "q"))
                };
                individualSymbolMiniTicker(JTO_STRING(value, "s"), value["E"].GetUint64(), smmt);
            }
            else if (type == "24hrTicker")
            {
                StreamIndividualSymbolTicker sst {
                    .priceChange                 = std::stod(JTO_STRING(value, "p")),
                    .priceChangePercent          = std::stod(JTO_STRING(value, "P")),
                    .weightedAveratePrice        = std::stod(JTO_STRING(value, "w")),
                    .lastPrice                   = std::stod(JTO_STRING(value, "c")),
                    .lastPriceQuantity           = std::stod(JTO_STRING(value, "Q")),
                    .openPrice                   = std::stod(JTO_STRING(value, "o")),
                    .highPrice                   = std::stod(JTO_STRING(value, "h")),
                    .lowPrice                    = std::stod(JTO_STRING(value, "l")),
                    .totalTradedBaseAssetVolume  = std::stod(JTO_STRING(value, "v")),
                    .totalTradedQuoteAssetVolume = std::stod(JTO_STRING(value, "q")),
                    .statisticsOpenTime          = value["O"].GetInt64(),
                    .statisticsCloseTime         = value["C"].GetInt64(),
                    .firstTreadeId               = value["F"].GetInt64(),
                    .lastTradeId                 = value["L"].GetInt64(),
                    .totalNumberOfTrades         = value["n"].GetUint64()
                };
                individualSymbolTicker(JTO_STRING(value, "s"), value["E"].GetUint64(), sst);
            }
            else if (type == "bookTicker")
            {
                StreamBookTicker sbt {
                    .transactionTime = value["T"].GetUint64(),
                    .bestBidPice     = std::stod(JTO_STRING(value, "b")),
                    .bestBidQty      = std::stod(JTO_STRING(value, "B")),
                    .bestAskPice     = std::stod(JTO_STRING(value, "a")),
                    .bestAskQty      = std::stod(JTO_STRING(value, "A")),
                };
                bookTicker(JTO_STRING(value, "s"), value["E"].GetUint64(), sbt);
            }
            else if (type == "forceOrder")
            {
                const auto &fod           = value["o"].GetObject();

                const std::string sSide   = JTO_STRING(fod, "S");
                const std::string sType   = JTO_STRING(fod, "T");
                const std::string sTIF    = JTO_STRING(fod, "f");
                const std::string sStatus = JTO_STRING(fod, "X");

                StreamLiquidationOrder slo {
                    .side = [&sSide]() {
                        if (sSide == "SELL")
                            return OrderSide::sell;
                        return OrderSide::buy;
                    }(),
                    .type = [&sType]() {
                        if (sType == "LIMIT")
                            return OrderType::limit;
                        else if (sType == "MARKET")
                            return OrderType::market;
                        else if (sType == "STOP_LOSS")
                            return OrderType::stopLoss;
                        else if (sType == "STOP_LOSS_LIMIT")
                            return OrderType::stopLossLimit;
                        else if (sType == "TAKE_PROFIT")
                            return OrderType::takeProfit;
                        else if (sType == "TAKE_PROFIT_LIMIT")
                            return OrderType::takeProfitLimit;
                        else if (sType == "LIMIT_MAKER")
                            return OrderType::limitMaker;
                        else if (sType == "STOP")
                            return OrderType::stop;
                        else if (sType == "STOP_MARKET")
                            return OrderType::stopMarket;
                        else if (sType == "STOP_PROFIT_MARKET")
                            return OrderType::takeProfitMarket;
                        else if (sType == "TRAILING_STOP_MARKET")
                            return OrderType::trailingStopMarket;
                        return OrderType::limit; // SHOULD NOT BE REACHED, unless API changes
                    }(),
                    .tif = [&sTIF]() {
                        if (sTIF == "GTC")
                            return OrderTimeInForce::GTC;
                        else if (sTIF == "IOC")
                            return OrderTimeInForce::IOC;
                        else if (sTIF == "FOK")
                            return OrderTimeInForce::FOK;
                        return OrderTimeInForce::GTC;
                    }(),
                    .originalQty  = std::stod(JTO_STRING(fod, "q")),
                    .price        = std::stod(JTO_STRING(fod, "p")),
                    .averagePrice = std::stod(JTO_STRING(fod, "ap")),
                    .status       = [&sStatus]() {
                        if (sStatus == "NEW")
                            return OrderStatus::newOrder;
                        else if (sStatus == "PARTIALLY_FILLED")
                            return OrderStatus::partiallyFilled;
                        else if (sStatus == "FILLED")
                            return OrderStatus::filled;
                        else if (sStatus == "CANCELED")
                            return OrderStatus::canceled;
                        else if (sStatus == "PENDING_CANCEL")
                            return OrderStatus::pendingCancel;
                        else if (sStatus == "REJECTED")
                            return OrderStatus::rejected;
                        return OrderStatus::expired;
                    }(),
                    .orderLastFilledQty        = std::stod(JTO_STRING(fod, "l")),
                    .orderFilledAccumulatedQty = std::stod(JTO_STRING(fod, "z")),
                    .tradeTime                 = fod["T"].GetUint64()
                };
                liquidationOrder(JTO_STRING(fod, "s"), value["E"].GetUint64(), slo);
            }
            else if (type == "depthUpdate")
            {
                StreamDepthUpdate sdp {
                    .transactionTime         = value["T"].GetUint64(),
                    .firstUpdateId           = value["U"].GetUint64(),
                    .finalUpdateId           = value["u"].GetUint64(),
                    .finalUpdateIdLastStream = value["pu"].GetUint64()
                };
                for (const auto &bids : value["b"].GetArray())
                    sdp.bids[JTO_STRING(bids, 0)] = JTO_STRING(bids, 1);
                for (const auto &asks : value["a"].GetArray())
                    sdp.asks[JTO_STRING(asks, 0)] = JTO_STRING(asks, 1);

                depthUpdate(JTO_STRING(value, "s"), value["E"].GetUint64(), sdp);
            }
            else if (type == "nav")
            {
                StreamBLVTInfo sbi;
                sbi.tokenIssued    = value["m"].GetDouble();
                sbi.BLVTNav        = value["n"].GetDouble();
                sbi.realLeverage   = value["l"].GetDouble();
                sbi.targetLeverage = value["t"].GetDouble();
                sbi.fundingRatio   = value["f"].GetDouble();
                for (const auto &baskets : value["b"].GetArray())
                    sbi.baskets[JTO_STRING(baskets, "s")] = baskets["n"].GetDouble();
                blvtInfo(JTO_STRING(value, "s"), value["E"].GetUint64(), sbi);
            }
            else if (type == "compositeIndex")
            {
                std::vector<StreamCompositionIndex> vsci;
                for (const auto &composition : value["c"].GetArray())
                {
                    StreamCompositionIndex sci {
                        .baseAsset          = JTO_STRING(composition, "b"),
                        .quoteAsset         = JTO_STRING(composition, "q"),
                        .weightInQuantity   = std::stod(JTO_STRING(composition, "")),
                        .weightInPercentage = std::stod(JTO_STRING(composition, "")),
                        .indexPrice         = std::stod(JTO_STRING(composition, ""))
                    };
                    vsci.push_back(sci);
                }
                compositeIndex(JTO_STRING(value, "s"), value["E"].GetUint64(), std::stod(JTO_STRING(value, "p")), vsci);
            }
        }
        else // STREAM IS AN ARRAY
        {
            std::vector<StreamMarkPriceAllMarket> vsmpam;
            std::multimap<std::string, std::pair<uint64_t, StreamMarketMiniTickers>> mmsmmt;
            std::multimap<std::string, std::pair<uint64_t, StreamMarketTickers>> mmsmt;
            for (const auto &streamArray : value.GetArray())
            {
                const std::string stype = JTO_STRING(streamArray, "e");
                if (stype == "markPriceUpdate")
                {
                    StreamMarkPriceAllMarket smpam {
                        .symbol           = JTO_STRING(streamArray, "s"),
                        .markPrice        = std::stod(JTO_STRING(streamArray, "p")),
                        .indexPrice       = std::stod(JTO_STRING(streamArray, "i")),
                        .estimSettlePrice = std::stod(JTO_STRING(streamArray, "P")),
                        .fundingRate      = std::stod(JTO_STRING(streamArray, "r")),
                        .nextFundingTime  = streamArray["T"].GetUint64(),
                        .eventTime        = streamArray["E"].GetUint64()
                    };
                    vsmpam.push_back(smpam);
                }
                else if (stype == "24hrMiniTicker")
                {
                    StreamMarketMiniTickers smmt {
                        .closePrice                  = std::stod(JTO_STRING(streamArray, "c")),
                        .openPrice                   = std::stod(JTO_STRING(streamArray, "o")),
                        .highPrice                   = std::stod(JTO_STRING(streamArray, "h")),
                        .lowPrice                    = std::stod(JTO_STRING(streamArray, "l")),
                        .totalTradedBaseAssetVolume  = std::stod(JTO_STRING(streamArray, "v")),
                        .totalTradedQuoteAssetVolume = std::stod(JTO_STRING(streamArray, "q"))
                    };
                    mmsmmt.insert({
                        JTO_STRING(value, "s"), {value["E"].GetUint64(), smmt}
                    });
                }
                else if (stype == "24hrTicker")
                {
                    StreamMarketTickers smt {
                        .priceChange                 = std::stod(JTO_STRING(value, "p")),
                        .priceChangePercent          = std::stod(JTO_STRING(value, "P")),
                        .weightedAveratePrice        = std::stod(JTO_STRING(value, "w")),
                        .lastPrice                   = std::stod(JTO_STRING(value, "c")),
                        .lastPriceQuantity           = std::stod(JTO_STRING(value, "Q")),
                        .openPrice                   = std::stod(JTO_STRING(value, "o")),
                        .highPrice                   = std::stod(JTO_STRING(value, "h")),
                        .lowPrice                    = std::stod(JTO_STRING(value, "l")),
                        .totalTradedBaseAssetVolume  = std::stod(JTO_STRING(value, "v")),
                        .totalTradedQuoteAssetVolume = std::stod(JTO_STRING(value, "q")),
                        .statisticsOpenTime          = value["O"].GetInt64(),
                        .statisticsCloseTime         = value["C"].GetInt64(),
                        .firstTreadeId               = value["F"].GetInt64(),
                        .lastTradeId                 = value["L"].GetInt64(),
                        .totalNumberOfTrades         = value["n"].GetUint64()
                    };
                    mmsmt.insert({
                        JTO_STRING(value, "s"), {value["E"].GetUint64(), smt}
                    });
                }
            }
            if (!vsmpam.empty())
                markPriceStreamAllMarket(vsmpam);
            if (!mmsmmt.empty())
                allMarketMiniTickers(mmsmmt);
            if (!mmsmt.empty())
                allMarketTickers(mmsmt);
        }
    } catch (const std::exception &ex)
    {
        fmt::print(fmt::fg(fmt::color::red), "JSON STREAM ERROR: ");
        fmt::print("JSON data could not be parsed: {}\n", ex.what());
    }
}

void trader::api::ws::WSFuturesBinanceAPI::aggregateTradeStream([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamAggregateTrade &as) { }
void trader::api::ws::WSFuturesBinanceAPI::markPriceStream([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamMarkPrice &smp) { }
void trader::api::ws::WSFuturesBinanceAPI::markPriceStreamAllMarket([[maybe_unused]] const std::vector<StreamMarkPriceAllMarket> &vsmpal) { }
void trader::api::ws::WSFuturesBinanceAPI::kline([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const BinanceTimeIntervals &interval, [[maybe_unused]] const Candlestick &cs) { }
void trader::api::ws::WSFuturesBinanceAPI::continuousKline([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const ContractType &ct, [[maybe_unused]] const BinanceTimeIntervals &interval, [[maybe_unused]] const Candlestick &cs) { }
void trader::api::ws::WSFuturesBinanceAPI::individualSymbolMiniTicker([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamIndividualSymbolMiniTicker &ticker) { }
void trader::api::ws::WSFuturesBinanceAPI::allMarketMiniTickers([[maybe_unused]] const std::multimap<std::string, std::pair<uint64_t, StreamMarketMiniTickers>> &mm) { }
void trader::api::ws::WSFuturesBinanceAPI::individualSymbolTicker([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamIndividualSymbolTicker &ticker) { }
void trader::api::ws::WSFuturesBinanceAPI::allMarketTickers([[maybe_unused]] const std::multimap<std::string, std::pair<uint64_t, StreamMarketTickers>> &mm) { }
void trader::api::ws::WSFuturesBinanceAPI::bookTicker([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamBookTicker &ticker) { }
void trader::api::ws::WSFuturesBinanceAPI::liquidationOrder([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamLiquidationOrder &slo) { }
void trader::api::ws::WSFuturesBinanceAPI::depthUpdate([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamDepthUpdate &sdp) { }
void trader::api::ws::WSFuturesBinanceAPI::blvtInfo([[maybe_unused]] const std::string &blvtName, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const StreamBLVTInfo &blvt) { }
void trader::api::ws::WSFuturesBinanceAPI::compositeIndex([[maybe_unused]] const std::string &symbol, [[maybe_unused]] const uint64_t &eventTime, [[maybe_unused]] const currency_t &price, [[maybe_unused]] const std::vector<StreamCompositionIndex> &sci) { }

int trader::api::ws::WSFuturesBinanceAPI::eventManager(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    auto client = reinterpret_cast<trader::api::ws::WSFuturesBinanceAPI *>(user);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif /*__clang__*/
    switch (reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lws_callback_on_writable(wsi);
            if (client)
            {
                client->m_running = true;
                client->connected();
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                std::string msg = std::string(reinterpret_cast<char *>(in), len);

                if (client)
                {
                    if (lws_frame_is_binary(wsi))
                    {
                        fmt::print("LWS received binary data and was ignored.\n");
                    }
                    else
                        client->receivedData(std::string(reinterpret_cast<char *>(in), len));
                }
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            break;

        case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CLOSED:
            if (client)
            {
                // Set proper flags to allow reconnection
                client->m_running   = false;
                client->m_terminate = true;

                client->close();
            }
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if (client)
            {
                client->m_running   = false;
                client->m_terminate = true;
                client->connectionError();
            }
            break;

        default:
            break;
    }

#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/

    return 0;
}

std::string trader::api::ws::WSFuturesBinanceAPI::constructEndPointFromList(const std::vector<std::string> &subscriptions) noexcept
{
    std::string ret;
    int count = 0;
    for (auto &sub : subscriptions)
    {
        ret += sub + "/";
        ++count;
    }
    if (!ret.empty())
        ret.pop_back();
    if (count == 1)
        ret.insert(0, "/ws/");
    else if (count > 1)
        ret.insert(0, "/stream?streams=");
    else
        ret.clear();
    return ret;
}
