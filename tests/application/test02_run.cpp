#include "global.hpp"
#include "gtest/gtest.h"

TEST(eoepcaows_02, SimpleRun) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
}

TEST(eoepcaows_02, Get_entry_identifier) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      EXPECT_EQ("application_package_sample_app",
                entry->getPackageIdentifier());
    }
  }
}

TEST(eoepcaows_02, Get_Offerings) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      auto& offerings = entry->getOfferings();
      EXPECT_EQ(1, offerings.size());

      for (auto& offering : offerings) {
        auto& contents = offering->getContents();
        EXPECT_EQ(2, contents.size());
      }
    }
  }
}

TEST(eoepcaows_02, Get_ProcessDescription) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      auto& offerings = entry->getOfferings();
      EXPECT_EQ(1, offerings.size());

      for (auto& offering : offerings) {
        auto& processDescriptions = offering->getProcessDescription();
        EXPECT_EQ(1, processDescriptions.size());

        for (auto& pd : processDescriptions) {
          EXPECT_EQ("sample-app", pd->getIdentifier());
          EXPECT_EQ("A sample App", pd->getTitle());
          EXPECT_EQ("Sample abstract",
                    pd->getAbstract());
        }
      }
    }
  }
}

TEST(eoepcaows_02, Get_Inputs_description) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      auto& offerings = entry->getOfferings();
      EXPECT_EQ(1, offerings.size());

      for (auto& offering : offerings) {
        auto& processDescriptions = offering->getProcessDescription();
        EXPECT_EQ(1, processDescriptions.size());

        for (auto& pd : processDescriptions) {
          auto& inputs = pd->getInputs();
          EXPECT_EQ(4, inputs.size());

          for (auto& input : inputs) {
            if (input->getIdentifier() == "input_reference") {
              EXPECT_EQ("S2 references", input->getTitle());
              EXPECT_EQ(
                  "S2 input reference as a comma separated list of catalogue "
                  "references",
                  input->getAbstract());
              EXPECT_EQ("LiteralData", input->getType());
            }
          }
        }
      }
    }
  }
}

TEST(eoepcaows_02, Get_Output_description) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      auto& offerings = entry->getOfferings();
      EXPECT_EQ(1, offerings.size());

      for (auto& offering : offerings) {
        auto& processDescriptions = offering->getProcessDescription();
        EXPECT_EQ(1, processDescriptions.size());

        for (auto& pd : processDescriptions) {
          auto& outputs = pd->getOutputs();
          EXPECT_EQ(1, outputs.size());

          for (auto& output : outputs) {
            if (output->getIdentifier() == "result_osd") {
              EXPECT_EQ("OpenSearch Description to the Results",
                        output->getTitle());
              EXPECT_EQ("OpenSearch Description to the Results",
                        output->getAbstract());
              EXPECT_EQ("ComplexData", output->getType());
            }
          }
        }
      }
    }
  }
}

TEST(eoepcaows_02, Get_static_cast_BoundingBox_input) {
  auto util = std::make_unique<Util>();

  std::string path{util->getLibName()};
  EXPECT_TRUE(!path.empty());

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);
  std::unique_ptr<EOEPCA::OWS::OWSContext,
                  std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromFile(util->getOws1()), lib->releaseParameter);

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      auto& offerings = entry->getOfferings();
      EXPECT_EQ(1, offerings.size());

      for (auto& offering : offerings) {
        auto& processDescriptions = offering->getProcessDescription();
        EXPECT_EQ(1, processDescriptions.size());

        for (auto& pd : processDescriptions) {
          auto& inputs = pd->getInputs();
          EXPECT_EQ(4, inputs.size());

          for (auto& input : inputs) {
            if (input->getType() == "BoundingBoxData") {
              auto data =
                  dynamic_cast<EOEPCA::OWS::BoundingBoxData*>(input.get());

              if (data != nullptr) {
                EXPECT_EQ(3, data->getSupported().size());
                EXPECT_EQ("CRS = urn:ogc:def:crs:EPSG:6.6:4326",
                          data->getDefaultValue());
              }
            }
          }
        }
      }
    }
  }
}
