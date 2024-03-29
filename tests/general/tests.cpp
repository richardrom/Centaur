

#include "QtCore/qnamespace.h"
#include "QtGui/qcolor.h"
#include <Protocol.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <uuid.hpp>

#ifdef USE_THEME_TESTING
// This file is auto generated by cmake
#include "auto_theme/general.h"
#include <QLinearGradient>
#include <ThemeParser.hpp>
#endif /*USE_THEME_TESTING*/

TEST_CASE("UUID Construction")
{
    using namespace std::string_literals;
    using namespace Catch::Matchers;

    CHECK_THROWS_WITH(cen::uuid(""), ContainsSubstring("empty"));
    CHECK_THROWS_WITH(cen::uuid("12121212-1212-1212-1212-141414141414}"), ContainsSubstring("missing a bracket"));
    CHECK_THROWS_WITH(cen::uuid("{12121212-1212-1212-1212-141414141414"), ContainsSubstring("missing a bracket"));
    CHECK_THROWS_WITH(cen::uuid("{12121212-1212-1212-122-14141414114}"), ContainsSubstring("invalid size"));
    CHECK_THROWS_WITH(cen::uuid("{12121212-1212-1212-1223-14141414114ad}"), ContainsSubstring("invalid size"));
    CHECK_THROWS_WITH(cen::uuid("{6f121212-1212-1212-1224-014m41414114}"), ContainsSubstring("invalid character"));
    CHECK_THROWS_WITH(cen::uuid("{6f121212-1212 1212-1224-014b41414114}"), ContainsSubstring("wrong format"));
    CHECK_THROWS_WITH(cen::uuid("{6f12121-0-1212-1212-1224-014C-141114}"), ContainsSubstring("wrong format"));
    CHECK_NOTHROW(cen::uuid("{6f12121d-1212-1212-1224-014C14a11c14}"));
    CHECK_NOTHROW(cen::uuid("{abcdefAB-CDEF-0123-4567-890000000000}"));

    CHECK_NOTHROW(cen::uuid("12121212-1212-1212-1212-141414141414", false));
    CHECK_NOTHROW(cen::uuid("12121212-1212-1212-1212-141414141414", false));
    CHECK_THROWS_WITH(cen::uuid("12121212-1212-1212-122-14141414114", false), ContainsSubstring("invalid size"));
    CHECK_THROWS_WITH(cen::uuid("12121212-1212-1212-1223-14141414114ad", false), ContainsSubstring("invalid size"));
    CHECK_THROWS_WITH(cen::uuid("6f121212-1212-1212-1224-014m41414114", false), ContainsSubstring("invalid character"));
    CHECK_THROWS_WITH(cen::uuid("6f121212-1212 1212-1224-014b41414114", false), ContainsSubstring("wrong format"));
    CHECK_THROWS_WITH(cen::uuid("6f12121-0-1212-1212-1224-014C-141114", false), ContainsSubstring("wrong format"));
    CHECK_NOTHROW(cen::uuid("6f12121d-1212-1212-1224-014C14a11c14", false));
    CHECK_NOTHROW(cen::uuid("abcdefAB-CDEF-0123-4567-890000000000", false));

    CHECK_NOTHROW(cen::uuid::generate());

    cen::uuid uuid("{abcdefAB-CDEF-0123-4567-890000000000}");
    CHECK(uuid.to_string() == "{abcdefab-cdef-0123-4567-890000000000}");
    CHECK(uuid.to_string(false) == "abcdefab-cdef-0123-4567-890000000000");
    CHECK(uuid.to_string(true, true) == "{ABCDEFAB-CDEF-0123-4567-890000000000}");
    CHECK(uuid.to_string(false, true) == "ABCDEFAB-CDEF-0123-4567-890000000000");

    cen::uuid bytes { 0xabcdefAB, 0xcdef, 0x0123, 0x4567, 0x89, 0x10, 0xcc, 0x90, 0x34, 0x35, true };
    CHECK(bytes == "{abcdefab-cdef-0123-4567-8910cc903435}"s);

    cen::uuid bytes_endianness { 0xabcdefAB, 0xcdef, 0x0123, 0x4567, 0x89, 0x10, 0xcc, 0x90, 0x34, 0x35, false };
    CHECK(bytes_endianness == "{abefcdab-efcd-2301-6745-8910cc903435}"s);
}

TEST_CASE("UUID maps")
{
    std::map<cen::uuid, int> l;

    cen::uuid i1 { "{abcdefAB-CDEF-0123-4567-890000000000}" };
    cen::uuid i2 { "{6f12121d-1212-1212-1224-014C14a11c14}" };

    l[i1] = 0x3433;
    l[i2] = 0x56c1;

    CHECK(l[i1] == 0x3433);
    CHECK(l[i2] == 0x56c1);
}

TEST_CASE("UUID unordered_maps")
{
    std::unordered_map<cen::uuid, int> l;

    cen::uuid i1 { "{abcdefAB-CDEF-0123-4567-890000000000}" };
    cen::uuid i2 { "{6f12121d-1212-1212-1224-014C14a11c14}" };

    l[i1] = 0x3433;
    l[i2] = 0x56c1;

    CHECK(l[i1] == 0x3433);
    CHECK(l[i2] == 0x56c1);
}

TEST_CASE("UUID unordered_maps no random collisions")
{
    std::unordered_map<cen::uuid, int> l;

    int col = 0;
    for (uint i = 0; i < 1'000'000; ++i) {
        auto id = cen::uuid::generate();
        if (auto iter = l.find(id); iter != l.end()) {
            ++col;
            std::cout << id.to_string() << "<-->" << iter->first.to_string() << "\n";
        }
        else
            l[id] = 0x12;
    }
    CHECK(col == 0);
}

TEST_CASE("UUID Benchmark")
{
    BENCHMARK_ADVANCED("UUID Constructor")
    (Catch::Benchmark::Chronometer meter)
    {
        std::vector<Catch::Benchmark::storage_for<cen::uuid>> uuids(static_cast<std::size_t>(meter.runs()));
        meter.measure([&](std::size_t i) { uuids[i].construct("{6f12121d-1212-1212-1224-014C14a11c14}"); });
    };

    BENCHMARK_ADVANCED("UUID Constructor integers endian=true")
    (Catch::Benchmark::Chronometer meter)
    {
        std::vector<Catch::Benchmark::storage_for<cen::uuid>> uuids(static_cast<std::size_t>(meter.runs()));
        meter.measure([&](std::size_t i) { uuids[i].construct(0xabcdefAB, 0xcdef, 0x0123, 0x4567, 0x89, 0x10, 0xcc, 0x90, 0x34, 0x35, true); });
    };

    BENCHMARK_ADVANCED("UUID Constructor integers endian=false")
    (Catch::Benchmark::Chronometer meter)
    {
        std::vector<Catch::Benchmark::storage_for<cen::uuid>> uuids(static_cast<std::size_t>(meter.runs()));
        meter.measure([&](std::size_t i) { uuids[i].construct(0xabcdefAB, 0xcdef, 0x0123, 0x4567, 0x89, 0x10, 0xcc, 0x90, 0x34, 0x35, false); });
    };

    BENCHMARK("UUID Generator mt19937")
    {
        [[maybe_unused]] auto uuid = cen::uuid::generate();
    };

    BENCHMARK("UUID Generator and stringify mt19937_64")
    {

        [[maybe_unused]] auto string = cen::uuid::generate().to_string();
    };
}

TEST_CASE("Field Protocols Types and names")
{
    using namespace CENTAUR_NAMESPACE;
    protocol::Field<int> fd("double");
    CHECK(fd.name() == "double");
    STATIC_REQUIRE(std::is_same_v<protocol::Field<int>::field_type, int>);
    fd() = 123;
    CHECK(fd() == 123);
}

TEST_CASE("Field Protocols")
{
    using namespace CENTAUR_NAMESPACE;
    protocol::ProtocolJSONGenerator pjg;
    protocol::Field<double> fd { "double" };
    protocol::Field<bool> fbf { "f_boolean" };
    protocol::Field<bool> fbt { "t_boolean" };
    protocol::Field<int64_t> fi { "signed64" };
    protocol::Field<uint32_t> fu { "unsigned" };
    protocol::Field<std::string> fs { "string" };

    fd()  = 32.455;
    fbf() = false;
    fbt() = true;
    fi()  = -123;
    fu()  = 124;
    fs()  = "this is a string";

    pjg.addField(std::addressof(fd));
    pjg.addField(std::addressof(fbf));
    pjg.addField(std::addressof(fbt));
    pjg.addField(std::addressof(fi));
    pjg.addField(std::addressof(fu));
    pjg.addField(std::addressof(fs));

    auto str = pjg.getJSON();

    CHECK(str == R"({"data":{"double":32.45500000,"f_boolean":false,"t_boolean":true,"signed64":-123,"unsigned":124,"string":"this is a string"}})");
}

TEST_CASE("Field protocols from JSON")
{
    using namespace CENTAUR_NAMESPACE;
    protocol::ProtocolJSONGenerator pjg;
    protocol::Field<double> fd { "double" };
    protocol::Field<bool> fb { "boolean" };
    protocol::Field<int64_t> fi { "signed64" };
    protocol::Field<uint32_t> fu { "unsigned" };
    protocol::Field<std::string> fs { "string" };

    pjg.addField(std::addressof(fd));
    pjg.addField(std::addressof(fb));
    pjg.addField(std::addressof(fi));
    pjg.addField(std::addressof(fu));
    pjg.addField(std::addressof(fs));

    CHECK_THROWS(pjg.fromJSON(R"({"data":{"double":1.3,"boolean":false,"signed64":-10,"unsigned":50,"string":"this is a string"})"));
    CHECK_NOTHROW(pjg.fromJSON(R"({"data":{"double":1.3,"boolean":false,"signed64":-10,"unsigned":50,"string":"this is a string"}})"));

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif /*__clang__*/
    CHECK(fd() == 1.3);
#ifdef __clang__
#pragma clang diagnostic pop
#endif /*__clang__*/
    CHECK(fb() == false);
    CHECK(fi() == -10);
    CHECK(fu() == 50);
    CHECK(fs() == "this is a string");
}

TEST_CASE("Communication JSON Protocols. Accept Connection")
{
    using namespace CENTAUR_NAMESPACE;
    protocol::message::Protocol_AcceptConnection pac;

    pac.uuid() = "{afc31b50-481a-0dce-40b5-4bd59f1118ad}";
    pac.name() = "Connection test";

    CHECK(pac.json() == R"({"data":{"uuid":"{afc31b50-481a-0dce-40b5-4bd59f1118ad}","name":"Connection test"}})");

    protocol::message::Protocol_AcceptConnection pacRes;
    pacRes.fromJson(pac.json().c_str());

    CHECK(pacRes.uuid() == "{afc31b50-481a-0dce-40b5-4bd59f1118ad}");
    CHECK(pacRes.name() == "Connection test");
}

TEST_CASE("Communication JSON Protocols. Accepted Connection")
{
    using namespace CENTAUR_NAMESPACE;

    protocol::message::Protocol_AcceptedConnection padc;
    padc.uuid() = "{afc31b50-481a-0dce-40b5-4bd59f1118ad}";

    CHECK(padc.json() == R"({"data":{"uuid":"{afc31b50-481a-0dce-40b5-4bd59f1118ad}"}})");

    protocol::message::Protocol_AcceptedConnection padcRes;
    padcRes.fromJson(padc.json().c_str());
    CHECK(padcRes.uuid() == "{afc31b50-481a-0dce-40b5-4bd59f1118ad}");
}

TEST_CASE("Protocol: Packed ProtocolHedare")
{
    // JUST TO CHECK THAT THERE IS NO PADDING ON THE STRUCT
    const std::size_t size = sizeof(cen::protocol::ProtocolHeader);
    CHECK(size == 104);
}

TEST_CASE("Protocol: HASH Generation")
{
    // Text taken from https://en.wikipedia.org/wiki/SHA-2
    const std::string text = "SHA-2 (Secure Hash Algorithm 2) is a set of cryptographic hash functions designed by the United States National Security Agency (NSA) and first published in 2001.[3][4] "
                             "They are built using the Merkle–Damgård construction, from a one-way compression function itself built using the Davies–Meyer structure from a specialized block cipher.";

    // Hash calculated in: https://emn178.github.io/online-tools/sha256.html
    const int8_t hash[65] = { "b15875a8710ab7ac96fb5b5623041fa46a3acd015ddf05c71b6b98f847f944b8" };

    cen::protocol::ProtocolHeader ph {};
    cen::protocol::Generator::hash(&ph, text);

    // Test basic
    CHECK(memcmp(hash, ph.hash, sizeof(ph.hash) / sizeof(ph.hash[0])) == 0);

    // Test the Generator::testHash
    cen::protocol::ProtocolHeader test {};
    memcpy(test.hash, hash, cen::protocol::g_hashSize);
    CHECK(cen::protocol::Generator::testHash(&test, text));

    const std::string textModify = "HA-2 (Secure Hash Algorithm 2) is a set of cryptographic hash functions designed by the United States National Security Agency (NSA) and first published in 2001.[3][4] "
                                   "They are built using the Merkle–Damgård construction, from a one-way compression function itself built using the Davies–Meyer structure from a specialized block cipher.";

    // Must return false
    CHECK(!cen::protocol::Generator::testHash(&test, textModify));
}

TEST_CASE("Protocol: Compression/Decompression")
{
    // Text taken from https://en.wikipedia.org/wiki/SHA-2
    const std::string text = "SHA-2 (Secure Hash Algorithm 2) is a set of cryptographic hash functions designed by the United States National Security Agency (NSA) and first published in 2001.[3][4] "
                             "They are built using the Merkle–Damgård construction, from a one-way compression function itself built using the Davies–Meyer structure from a specialized block cipher.";

    std::vector<uint8_t> compressed;

    cen::protocol::ProtocolHeader test {};

    test.size = text.size(); // Must be set for the decompression
    cen::protocol::Generator::compress(&test, text, compressed, 9);

    std::string decompressed = cen::protocol::Generator::decompress(&test, static_cast<const uint8_t *>(compressed.data()), compressed.size());

    CHECK(decompressed == text);
}

TEST_CASE("Protocol: Send/Receive NO Compression")
{

    CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_AcceptConnection pac;
    pac.name() = "name";
    pac.uuid() = "uuid";

    CENTAUR_PROTOCOL_NAMESPACE::Protocol send;
    CENTAUR_PROTOCOL_NAMESPACE::Generator::generate(&send, 10, &pac, false);

    cen::protocol::ProtocolHeader header {};
    auto receive = CENTAUR_PROTOCOL_NAMESPACE::Generator::getData(&header, send.get(), send.getSize(), 100000);

    CHECK(receive == pac.json());
}

TEST_CASE("Protocol: Send/Receive With Compression")
{

    CENTAUR_PROTOCOL_NAMESPACE::message::Protocol_AcceptConnection pac;
    pac.name() = "name";
    pac.uuid() = "uuid";

    CENTAUR_PROTOCOL_NAMESPACE::Protocol send;
    CENTAUR_PROTOCOL_NAMESPACE::Generator::generate(&send, 10, &pac, true);

    cen::protocol::ProtocolHeader header {};
    auto receive = CENTAUR_PROTOCOL_NAMESPACE::Generator::getData(&header, send.get(), send.getSize(), 100000);

    CHECK(receive == pac.json());
}

TEST_CASE("Protocol: Encryption")
{
    CENTAUR_PROTOCOL_NAMESPACE::Encryption ec;

    // CHECK_NOTHROW(ec.loadPrivateKey("/Volumes/RicardoESSD/Projects/Centaur/local/Resources/Private/{85261bc6-8f92-57ca-802b-f08b819031db}.pem"));

    const std::string pl = "asdasdas";

    try {

        const auto e = CENTAUR_PROTOCOL_NAMESPACE::Encryption::EncryptAES(pl, "keyasdasdasdasddkeyasdasdasdasdd", "3331231231231231");

        const auto d = CENTAUR_PROTOCOL_NAMESPACE::Encryption::DecryptAES(e, "keyasdasdasdasddkeyasdasdasdasdd", "3331231231231231");

        CHECK(pl == d);
    } catch (const std::exception &ex) {
        std::string d = ex.what();
    }
}

#ifdef USE_THEME_TESTING
static constexpr const char *const test_path = { THEME_TESTING_LOCAL_FILES };

TEST_CASE("Theme File Loader - Basic Elements")
{
    const std::string path { test_path };
    theme::ThemeParser parser;
    SECTION("Theme node parent")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-theme-node.xml"), Catch::Matchers::ContainsSubstring("parent"));
    }

    SECTION("Theme with no name")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-name-0.xml"), Catch::Matchers::ContainsSubstring("name"));
    }

    SECTION("Theme with no renders")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-name-1.xml"), Catch::Matchers::ContainsSubstring("renders"));
    }

    SECTION("Renders with invalid node")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-renders-types.xml"), Catch::Matchers::ContainsSubstring("unknown render"));
    }

    SECTION("Renders with invalid hint")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-render-hint.xml"), Catch::Matchers::ContainsSubstring("invalid render hint"));
    }

    SECTION("Valid name and renders")
    {
        CHECK_NOTHROW(parser.loadTheme(path + "/name-render.xml"));
        CHECK(parser.themeScheme == "theme name");
        // As of Qt6.4 these are the values for QPainter::RenderHints
        CHECK(parser.renderHints == (0x01 | 0x02 | 0x04 | 0x08 | 0x40 | 0x80));
    }

    SECTION("Invalid colors")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-color-0.xml"), Catch::Matchers::ContainsSubstring("not recognized as valid"));
    }

    SECTION("Invalid color name")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-color-1.xml"), Catch::Matchers::ContainsSubstring("color has no name"));
    }

    SECTION("Color already exists")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-color-2.xml"), Catch::Matchers::ContainsSubstring("already exists"));
    }

    SECTION("Invalid color value")
    {
        CHECK_THROWS_WITH(parser.loadTheme(path + "/error-color-3.xml"), Catch::Matchers::ContainsSubstring("color node is not valid"));
    }

    SECTION("Color values")
    {
        CHECK_NOTHROW(parser.loadTheme(path + "/colors.xml"));

        CHECK(parser.scheme.colors["color0"] == QColor(0, 0, 0, 255));
        CHECK(parser.scheme.colors["color1"] == QColor(255, 0, 0, 255));

        QColor color2_;
        color2_.setRgbF(0.5f, 0.f, 0.f, 1.0f);
        CHECK(parser.scheme.colors["color2"] == color2_);
        CHECK(parser.scheme.colors["color3"] == QColor(0, 255, 0, 255));

        QColor color4_;
        color4_.setRgbF(0.f, 0.5f, 0.f, 1.0f);
        CHECK(parser.scheme.colors["color4"] == color4_);
        CHECK(parser.scheme.colors["color5"] == QColor(0, 0, 255, 255));
        QColor color6_;
        color6_.setRgbF(0.f, 0.0f, 0.5f, 1.0f);
        CHECK(parser.scheme.colors["color6"] == color6_);

        CHECK(parser.scheme.colors["color7"] == QColor(128, 90, 43, 56));

        QColor color8_;
        color8_.setRgbF(0.3f, 0.89f, 0.2f, .1f);
        CHECK(parser.scheme.colors["color8"] == color8_);

        QColor color9_;
        color9_.setRgbF(0.5f, 0.f, 0.f, .6f);
        color9_.setGreen(12);
        color9_.setBlue(44);
        CHECK(parser.scheme.colors["color9"] == color9_);
    }

    SECTION("Brushes")
    {
        CHECK_NOTHROW(parser.loadTheme(path + "/brushes.xml"));

        {
            const QBrush &brush = parser.scheme.brushes["brush0"];

            CHECK(brush.color() == QColor(255, 255, 255));
            CHECK(brush.style() == Qt::BrushStyle::SolidPattern);
        }

        {
            const QBrush &brush = parser.scheme.brushes["brush1"];

            CHECK(brush.color() == QColor(0, 0, 0));
            CHECK(brush.style() == Qt::BrushStyle::DiagCrossPattern);

            const QBrush &linear = parser.scheme.brushes["gradient0"];

            const auto *linearGradient = static_cast<const QLinearGradient *>(linear.gradient());
            CHECK(linearGradient->spread() == QGradient::Spread::ReflectSpread);
            CHECK(linearGradient->start() == QPointF { 0.0, 0.0 });
            CHECK(linearGradient->finalStop() == QPointF { 0.5, 0.5 });

            const QGradientStops linStops {
                QGradientStop {0.0, QColor(255,   0,   0)},
                QGradientStop {0.5,   QColor(0,   0, 255)},
                QGradientStop {1.0,   QColor(0, 255,   0)}
            };

            CHECK(linearGradient->stops() == linStops);
        }

        {
            const QBrush &radial       = parser.scheme.brushes["gradient1"];
            const auto *radialGradient = static_cast<const QRadialGradient *>(radial.gradient());

            CHECK(radialGradient->center() == QPointF { 0.5, 0.5 });
            CHECK(radialGradient->focalPoint() == QPointF { 0.2, 0.2 });
            CHECK(qFuzzyCompare(radialGradient->radius(), 3.14));
            CHECK(qFuzzyCompare(radialGradient->focalRadius(), 2.718));

            const QGradientStops radStops {
                QGradientStop {0.2, QColor(255,   0,   0)},
                QGradientStop {0.4,   QColor(0,   0, 255)},
                QGradientStop {0.6,   QColor(0, 255,   0)}
            };

            CHECK(radialGradient->stops() == radStops);
        }

        {
            const QBrush &conical       = parser.scheme.brushes["gradient2"];
            const auto *conicalGradient = static_cast<const QConicalGradient *>(conical.gradient());

            CHECK(conicalGradient->center() == QPointF(0.1, 0.8));
            CHECK(qFuzzyCompare(conicalGradient->angle(), 45.0));

            const QGradientStops conStops {
                QGradientStop {0.3, QColor(255,   0,   0)},
                QGradientStop {0.6,   QColor(0,   0, 255)},
                QGradientStop {0.9,   QColor(0, 255,   0)}
            };

            CHECK(conicalGradient->stops() == conStops);
        }
    }

    SECTION("Pens")
    {
        CHECK_NOTHROW(parser.loadTheme(path + "/pens.xml"));

        {
            const QPen &pen = parser.scheme.pens["pen0"];

            const QBrush &brushPen = pen.brush();
            CHECK(brushPen.color() == QColor(255, 255, 255));
            CHECK(brushPen.style() == Qt::BrushStyle::SolidPattern);

            CHECK(qFuzzyCompare(pen.widthF(), 2.2));
            CHECK(pen.style() == Qt::PenStyle::DashDotLine);
            CHECK(pen.capStyle() == Qt::PenCapStyle::FlatCap);
            CHECK(pen.joinStyle() == Qt::PenJoinStyle::MiterJoin);
            CHECK(qFuzzyCompare(pen.miterLimit(), 12.3));
        }

        {
            const QPen &pen = parser.scheme.pens["pen1"];

            const QBrush &brushPen = pen.brush();
            CHECK(brushPen.color() == QColor(255, 255, 255));
            CHECK(brushPen.style() == Qt::BrushStyle::SolidPattern);

            CHECK(qFuzzyCompare(pen.widthF(), 3.0));
            CHECK(pen.style() == Qt::PenStyle::CustomDashLine);
            CHECK(pen.capStyle() == Qt::PenCapStyle::RoundCap);
            CHECK(pen.joinStyle() == Qt::PenJoinStyle::BevelJoin);

            const QList<qreal> pattern { 0.3, 2.2, 5.1, 3.7 };
            CHECK(pen.dashPattern() == pattern);
        }
    }
}

#endif
