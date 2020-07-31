#include "global.hpp"
#include "gtest/gtest.h"

#include "httpfuntions.hpp"


void  printResults(const std::unique_ptr<EOEPCA::OWS::Param>& input){
  std::cout << "--------------------------\n";
  std::cout << "getIdentifier: " << input->getIdentifier() << "\n";
  std::cout << "getType: " << input->getType() << "\n";
  std::cout << "--------------------------\n";
  std::cout << "getTitle: " << input->getTitle() << "\n";
  std::cout << "getAbstract: " << input->getAbstract() << "\n";

  std::cout << "getMinOccurs: " << input->getMinOccurs() << "\n";
  std::cout << "getMaxOccurs: " << input->getMaxOccurs() << "\n";


  if (input->getType() == "BoundingBoxData") {
    auto data =
        dynamic_cast<EOEPCA::OWS::BoundingBoxData*>(input.get());

  } else if (input->getType() == "ComplexData") {
    auto data = dynamic_cast<EOEPCA::OWS::ComplexData*>(input.get());

    std::cout << "getMaximumMegabytes: "
              << data->getMaximumMegabytes() << "\n";

    std::cout << "getSupported: \n";
    for (auto& supp : data->getSupported()) {
      std::cout << "\tgetEncoding: " << supp->getEncoding() << "\n";
      std::cout << "\tgetMimeType: " << supp->getMimeType() << "\n";
      std::cout << "\tgetSchema: " << supp->getSchema() << "\n";
      std::cout << "\t\n";
    }

    std::cout << "getDefaultSupported: \n";
    auto& supp2 = data->getDefaultSupported();
    std::cout << "\tgetEncoding: " << supp2->getEncoding() << "\n";
    std::cout << "\tgetMimeType: " << supp2->getMimeType() << "\n";
    std::cout << "\tgetSchema: " << supp2->getSchema() << "\n";
    std::cout << "\t\n";
  }
  if (input->getType() == "LiteralData") {
    auto data = dynamic_cast<EOEPCA::OWS::LiteralData*>(input.get());

    if ("int_parameter"==data->getIdentifier()){
      EXPECT_EQ(data->getTitle(),"Title of int_parameter");
      EXPECT_EQ(data->getAbstract(),"Abstract describing int_parameter");
      EXPECT_EQ(data->getDataType(),"integer");
      EXPECT_EQ(data->getMinOccurs(),1);
      EXPECT_EQ(data->getMaxOccurs(),1);
    }

    if ("string_parameter"==data->getIdentifier()){
      EXPECT_EQ(data->getTitle(),"Title of string_parameter");
      EXPECT_EQ(data->getAbstract(),"Abstract describing string_parameter");
      EXPECT_EQ(data->getDataType(),"string");
      EXPECT_EQ(data->getMinOccurs(),1);
      EXPECT_EQ(data->getMaxOccurs(),1);
      EXPECT_EQ(data->getDefaultValue(),"CIAO a te");
    }

    if ("string_parameter_arr"==data->getIdentifier()){
      EXPECT_EQ(data->getTitle(),"Title of string_parameter_arr");
      EXPECT_EQ(data->getAbstract(),"Abstract describing string_parameter_arr");
      EXPECT_EQ(data->getDataType(),"string");
      EXPECT_EQ(data->getMinOccurs(),1);
      EXPECT_EQ(data->getMaxOccurs(),999);
    }

    if ("string_with_options_parameter"==data->getIdentifier()){
      EXPECT_EQ(data->getTitle(),"Title of string_with_options_parameter");
      EXPECT_EQ(data->getDataType(),"string");
      EXPECT_EQ(data->getMinOccurs(),1);
      EXPECT_EQ(data->getMaxOccurs(),1);
      EXPECT_EQ(data->getAllowedValues().size(),3);
      int count{0};
      for(auto&v:data->getAllowedValues()){
        if (v=="‘option1’"||v=="‘options2’"||v=="‘option3’" ){
          count++;
        }
      }
      EXPECT_EQ(count,3);
    }




    std::cout << "getDataType: " << data->getDataType() << "\n";
    std::cout << "getAllowedValuesSize: "
              << data->getAllowedValues().size() << "\n";
    for (auto& a : data->getAllowedValues()) {
      std::cout << "AllowedValue: " << a << "\n";
    }

    std::cout << "getDefaultValue: " << data->getDefaultValue() <<"\n";

  }
  std::cout << "--------------------------\n";
}

TEST(eoepcaows_02, inputs_list_from_catalog) {
  auto util = std::make_unique<Util>();
  std::string path{util->getLibName()};

  std::cout << "start download OWS..."  <<"\n";
  std::string fileBuffer{""};
  getFromWeb(fileBuffer,"https://catalog.terradue.com/eoepca-services/search?uid=test_entites");

  std::cout << "CWL: \n";
  std::cout << fileBuffer << "\n\n\n\n";


  std::cout <<"lib: "<< path << "\n";

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);

  EXPECT_TRUE(lib->IsValid());

  if(!lib->IsValid()){
    std::cout << "EOEPCAows lib not valid";
    return;
  }

  std::unique_ptr<EOEPCA::OWS::OWSContext,
      std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromMemory(fileBuffer.c_str(),fileBuffer.size()), lib->releaseParameter);


  std::cout << "parser... end"  <<"\n";

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      for (auto& off : entry->getOfferings()) {
        for (auto& processDescription : off->getProcessDescription()) {

          std::cout << "----------OUTPUTS------------------\n";
          for (auto& input : processDescription->getInputs()) {
            printResults(input);
          }

          std::cout << "----------OUTPUTS------------------\n";
          for (auto& input : processDescription->getOutputs()) {
            printResults(input);
          }
        }
      }
    }
  }
}

TEST(eoepcaows_02, inputs_list_from_catalog_ref) {
  auto util = std::make_unique<Util>();
  std::string path{util->getLibName()};

  std::cout << "start download OWS..."  <<"\n";
  std::string fileBuffer{""};
  getFromWeb(fileBuffer,"https://catalog.terradue.com/eoepca-services/search?uid=test_entites_ref");

  std::cout << "CWL: \n";
  std::cout << fileBuffer << "\n\n\n\n";


  std::cout <<"lib: "<< path << "\n";

  auto lib = std::make_unique<EOEPCA::EOEPCAows>(path);

  EXPECT_TRUE(lib->IsValid());

  if(!lib->IsValid()){
    std::cout << "EOEPCAows lib not valid";
    return;
  }

  std::unique_ptr<EOEPCA::OWS::OWSContext,
      std::function<void(EOEPCA::OWS::OWSContext*)>>
      ptrContext(lib->parseFromMemory(fileBuffer.c_str(),fileBuffer.size()), lib->releaseParameter);


  std::cout << "parser... end"  <<"\n";

  EXPECT_NE(nullptr, ptrContext.get());
  if (ptrContext) {
    auto& entries = ptrContext->getEntries();
    EXPECT_EQ(1, entries.size());

    for (auto& entry : entries) {
      for (auto& off : entry->getOfferings()) {
        for (auto& processDescription : off->getProcessDescription()) {

          std::cout << "----------OUTPUTS------------------\n";
          for (auto& input : processDescription->getInputs()) {
            printResults(input);
          }

          std::cout << "----------OUTPUTS------------------\n";
          for (auto& input : processDescription->getOutputs()) {
            printResults(input);
          }

        }
      }
    }
  }
}

