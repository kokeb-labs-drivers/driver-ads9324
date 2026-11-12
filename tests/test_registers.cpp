/**
 * @file test_registers.cpp
 * @brief The register map against the model: 116 registers, reset values,
 * banks, the mirrored layout.
 */
#include "mock_ads9324.hpp"
#include "test_harness.hpp"

using namespace ads9324;
using ads9324_test::Ads9324Model;

TEST_CASE("T04 every documented register of the driver is documented in the "
          "model, with the same "
          "reset value") {
  Ads9324Model model;
  const uint8_t codes[3] = {1, 2, 4};
  int documented = 0;
  for (uint8_t b = 0; b < 3; ++b) {
    for (uint32_t a = 0; a < 64; ++a) {
      const auto bank = static_cast<reg::Bank>(b);
      const auto address = static_cast<uint8_t>(a);
      if (!reg::IsDocumented(bank, address)) {
        continue;
      }
      ++documented;
      if (address == reg::kGenCfg1) {
        continue; // the command register has no stored value
      }
      const uint16_t want = (bank == reg::Bank::Common) && (address == reg::kDiagCtrl)
                                ? 0x0100U // RESET_DETECT_FLAG reads 1 after a reset (Table 7-13)
                                : reg::ResetValue(bank, address);
      CHECK_EQ(model.Reg(codes[b], address), want);
    }
  }
  CHECK_EQ(documented, 116);
}

TEST_CASE("T04 addresses of the datasheet") {
  CHECK_EQ(reg::kGenCfg3, 0x0AU);
  CHECK_EQ(reg::kDeviceId, 0x21U);
  CHECK_EQ(reg::kDeviceStatus, 0x1DU);
  CHECK_EQ(reg::kOffset, 0x11U);
  CHECK_EQ(reg::kGain, 0x19U);
  CHECK_EQ(reg::kDwcTh, 0x22U);
  CHECK_EQ(reg::kTp, 0x2FU);
  CHECK_EQ(reg::kDwcFlag, 0x3EU);
  CHECK_EQ(reg::BankSelCode(reg::Bank::Common), 1U);
  CHECK_EQ(reg::BankSelCode(reg::Bank::Ain1To8), 2U);
  CHECK_EQ(reg::BankSelCode(reg::Bank::Ain9To16), 4U);
  CHECK(reg::IsReadOnly(reg::Bank::Common, reg::kDeviceId));
  CHECK(reg::IsReadOnly(reg::Bank::Ain9To16, reg::kDwcFlag));
  CHECK(!reg::IsShadowed(reg::Bank::Common, reg::kBankSel));
  CHECK(!reg::IsShadowed(reg::Bank::Common, reg::kGenCfg1));
  CHECK(reg::IsShadowed(reg::Bank::Common, reg::kGenCfg3));
}

TEST_CASE("T16 Locate and ChannelAt are inverse and agree with the model's layout") {
  Ads9324Model model;
  for (uint8_t ch = 0; ch < 16; ++ch) {
    const auto at = reg::Locate(ch);
    CHECK_EQ(static_cast<int>(at.local), model.LocalIndex(ch));
    CHECK_EQ(reg::ChannelAt(at.bank, at.local), ch);
    CHECK_EQ(at.bank == reg::Bank::Ain1To8, ch < 8);
  }
  CHECK_EQ(reg::PgaConfigAddress(reg::Locate(15).local),
           0x08U);                                       // AIN16 in the first register
  CHECK_EQ(reg::TpAddress(reg::Locate(8).local), 0x36U); // AIN9 in the last
}

TEST_CASE("T04 fields insert and extract only their own bits") {
  CHECK_EQ(reg::DoutLaneSel::Insert(0xFFFF, 1), 0xFFDFU);
  CHECK_EQ(reg::DoutLaneSel::Extract(0x0030), 3U);
  CHECK_EQ(reg::AdcNumSel::Extract(reg::AdcNumSel::Insert(0, 2)), 2U);
  CHECK_EQ(reg::MvgAvgLen::Insert(0, 0x1F), 0x0F00U); // masked to the width
}
