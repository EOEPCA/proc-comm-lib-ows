
#include "includes/parser.hpp"

#include <iostream>
#include <memory>
#include <optional>

#include "includes/macro.hpp"
#define echo std::cout

namespace EOEPCA {

const std::string& Parser::getName() const { return name; }

/// Users/rdirienzo/Project/t2pc/src/t2serverwps/kungfu-wps/schemas.opengis.net/wps/1.0.0/wpsDescribeProcess_response.xsd?#117

std::unique_ptr<OWS::Format> getFormat(xmlNode* nodeComplexDataFormat) {
  auto format = std::make_unique<OWS::Format>();

  FOR(f, nodeComplexDataFormat) {
    IF_XML_COMPARE(f->name, "MimeType") {
      format->setMimeType(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    }
    else IF_XML_COMPARE(f->name, "Encoding") {
      format->setEncoding(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    }
    else IF_XML_COMPARE(f->name, "Schema") {
      format->setSchema(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    }
  }

  return format;
}

[[nodiscard]] std::unique_ptr<OWS::ComplexData> parseComplexData(
    xmlNode* nodeComplexData) {
  auto complexData = std::make_unique<OWS::ComplexData>();

  xmlChar* maximumMegabytes =
      xmlGetProp(nodeComplexData, (const xmlChar*)"maximumMegabytes");

  if (maximumMegabytes) {
    complexData->setMaximumMegabytes(
        std::string(CHAR_BAD_CAST maximumMegabytes));
  }

  FOR(box, nodeComplexData) {
    IF_XML_COMPARE(box->name, "Default") {
      FOR(format, box) {
        auto mFrmat = getFormat(format);
        complexData->moveDefaultSupported(mFrmat);
      }
    }
    else IF_XML_COMPARE(box->name, "Supported") {
      FOR(format, box) {
        auto mFrmat = getFormat(format);
        complexData->moveAddSupported(mFrmat);
      }
    }
  }

  return std::move(complexData);
}

[[nodiscard]] std::unique_ptr<OWS::BoundingBoxData> parseBoundingBoxData(
    xmlNode* nodeBoundingBoxData) {
  auto boundingBoxData = std::make_unique<OWS::BoundingBoxData>();

  FOR(box, nodeBoundingBoxData) {
    IF_XML_COMPARE(box->name, "Supported") {
      FOR(crs, box) {
        IF_XML_COMPARE(crs->name, "CRS") {
          boundingBoxData->addSupportedValues(
              std::string(CHAR_BAD_CAST xmlNodeGetContent(crs)));
        }
      }
    }
    else IF_XML_COMPARE(box->name, "Default") {
      if (box->children &&
          !xmlStrcmp(box->children->name, (const xmlChar*)"CRS")) {
        boundingBoxData->setDefault(
            std::string(CHAR_BAD_CAST xmlNodeGetContent(box->children)));
      }
    }
  }

  return std::move(boundingBoxData);
}

[[nodiscard]] std::unique_ptr<OWS::LiteralData> parseLiteralData(
    xmlNode* nodeLiteralData) {
  auto literData = std::make_unique<OWS::LiteralData>();

  FOR(ldata, nodeLiteralData) {
    IS_CHECK(ldata, "AnyValue", XMLNS_WPS1) {}
    else IS_CHECK(ldata, "AllowedValues", XMLNS_OWS) {
      FOR(value, ldata) {
        IS_CHECK(value, "Value", XMLNS_OWS) {
          literData->addAllowedValues(
              std::string(CHAR_BAD_CAST xmlNodeGetContent(value)));
        }
      }
    }
    else IS_CHECK(ldata, "DataType", XMLNS_OWS) {
      literData->setDataType(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(ldata)));
    }
    else IS_CHECK(ldata, "DefaultValue", XMLNS_OWS) {
      literData->setDefault(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(ldata)));
    }
  }

  return std::move(literData);
}

void parseInput(xmlNode* nodeInput,
                std::unique_ptr<OWS::OWSParameter>& ptrParams) {
  // ptr definitions

  std::unique_ptr<OWS::Param> param{nullptr};

  auto ptrOccurs = std::make_unique<OWS::Occurs>();
  auto ptrDescriptor = std::make_unique<OWS::Descriptor>();

  xmlChar* minOccurs = xmlGetProp(nodeInput, (const xmlChar*)"minOccurs");
  xmlChar* maxOccurs = xmlGetProp(nodeInput, (const xmlChar*)"maxOccurs");

  if (minOccurs) {
    ptrOccurs->setMinOccurs(std::string(CHAR_BAD_CAST minOccurs));
  }

  if (maxOccurs) {
    ptrOccurs->setMaxOccurs(std::string(CHAR_BAD_CAST maxOccurs));
  }

  FOR(input, nodeInput) {
    IF_XML_COMPARE(input->ns->href, XMLNS_OWS) {
      IS_CHECK(input, "Identifier", XMLNS_OWS) {
        ptrDescriptor->setIdentifier(
            std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
      }
      else IS_CHECK(input, "Title", XMLNS_OWS) {
        ptrDescriptor->setTitle(
            std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
      }
      else IS_CHECK(input, "Abstract", XMLNS_OWS) {
        ptrDescriptor->setAbstract(
            std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
      }
    }

    IF_XML_COMPARE(input->ns->href, XMLNS_WPS1) {
      IS_CHECK(input, "LiteralData", XMLNS_WPS1) {
        // get literal data
        param = parseLiteralData(input);
      }
      else IS_CHECK(input, "BoundingBoxData", XMLNS_WPS1) {
        // get literal data
        param = parseBoundingBoxData(input);
      }
      else IS_CHECK(input, "ComplexData", XMLNS_WPS1) {
        param = parseComplexData(input);
      } else IF_XML_COMPARE(input->name,"ComplexData"){
            param = parseComplexData(input);
      }
      else {
        std::string err("Type ");
        err.append((char*)input->name);

        if (input->ns->href) {
          err.append(" with namespace ").append((char*)input->ns->href);
        }

        err.append(" not supported in this version!");
        throw std::runtime_error(err);
      }
    }
  }

  if (param) {
    *param << *ptrDescriptor << *ptrOccurs;
    ptrParams->addInput(param.release());
  }
}

void parseOutput(xmlNode* nodeOutput) {}

void parseProcessDescription(xmlNode* processDescription,
                             std::unique_ptr<OWS::OWSParameter>& ptrParams) {
  xmlChar* processVersion =
      xmlGetProp(processDescription, (const xmlChar*)"processVersion");
  if (processVersion) {
    ptrParams->setVersion(std::string(CHAR_BAD_CAST processVersion));
  }

  FOR(inner_cur_node, processDescription) {
    IS_CHECK(inner_cur_node, "Identifier", XMLNS_OWS) {
      ptrParams->setIdentifier(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    }
    else IS_CHECK(inner_cur_node, "Title", XMLNS_OWS) {
      ptrParams->setTitle(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    }
    else IS_CHECK(inner_cur_node, "Abstract", XMLNS_OWS) {
      ptrParams->setAbstract(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    }
    else IS_CHECK(inner_cur_node, "DataInputs", XMLNS_ATOM) {
      FOR(input, inner_cur_node) { parseInput(input, ptrParams); }
    }
    else IS_CHECK(inner_cur_node, "ProcessOutputs", XMLNS_ATOM) {
      echo << "ProcessOutputs*****\n";
    }
  }
}

void parseOffering(xmlNode* offering_node,
                   std::unique_ptr<OWS::OWSParameter>& ptrParams) {
  FOR(inner_cur_node, offering_node) {
    IS_CHECK(inner_cur_node, "content", XMLNS_OWC) {
      xmlChar* code = xmlGetProp(inner_cur_node, (const xmlChar*)"type");
      xmlChar* href = xmlGetProp(inner_cur_node, (const xmlChar*)"href");

      ptrParams->addContent(std::string(CHAR_BAD_CAST code),
                            std::string(CHAR_BAD_CAST href));
    }
    else IS_CHECK(inner_cur_node, "operation", XMLNS_OWC) {
      xmlChar* code = xmlGetProp(inner_cur_node, (const xmlChar*)"code");
      if (code) {
        IF_XML_COMPARE(code, "DescribeProcess") {
          if (inner_cur_node->children) {
            FOR(desc, inner_cur_node->children) {
              IS_CHECK(desc, "ProcessDescription", XMLNS_ATOM) {
                parseProcessDescription(desc, ptrParams);
              }
            }
          }
        }
      }
    }
  }
}

void parseEntry(xmlNode* entry_node,
                std::unique_ptr<OWS::OWSParameter>& ptrParams) {
  FOR(inner_cur_node, entry_node) {
    if (inner_cur_node->type == XML_COMMENT_NODE) {
      continue;
    } else if (inner_cur_node->type == XML_ELEMENT_NODE) {
      IS_CHECK(inner_cur_node, "identifier", XMLNS_PURL) {
        // set /entry/identifier/
        ptrParams->setPackageIdentifier(
            std::string((char*)xmlNodeGetContent(inner_cur_node)));
      }
      else IS_CHECK(inner_cur_node, "offering", XMLNS_OWC) {
        parseOffering(inner_cur_node, ptrParams);
      }
    }
  }
}

OWS::OWSParameter* Parser::parseXml(const char* bufferXml, int size) {
  int ret = 0;
  xmlDoc* doc = nullptr;
  xmlNode* root_element = nullptr;
  LIBXML_TEST_VERSION
  int option = XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS |
               XML_PARSE_NOBLANKS;

  std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> pDoc{
      xmlReadMemory(bufferXml, size, nullptr, nullptr, option), &xmlFreeDoc};

  auto PARAMETERs = std::make_unique<OWS::OWSParameter>();

  PARAMETERs->setTitle("THE TEST TITLE");

  try {
    if (pDoc == nullptr) {
      ret = 2;
    } else {
      root_element = xmlDocGetRootElement(pDoc.get());
      for (xmlNode* cur_node = root_element; cur_node;
           cur_node = cur_node->next) {
        if (cur_node->type == XML_COMMENT_NODE) {
          continue;
        }

        IS_CHECK(cur_node, "feed", XMLNS_ATOM) {
          if (cur_node->children != nullptr) {
            FOR(inner_entry_node, cur_node) {
              if (inner_entry_node->type == XML_COMMENT_NODE) {
                continue;
              } else if (inner_entry_node->type == XML_ELEMENT_NODE) {
                IS_CHECK(inner_entry_node, "entry", XMLNS_ATOM) {
                  parseEntry(inner_entry_node, PARAMETERs);
                }
              }
            }
          }
        }
      }
    }

  } catch (std::runtime_error& err) {
    std::cerr << err.what() << "\n";
    PARAMETERs.reset(nullptr);
  } catch (...) {
    std::cerr << "CATCH!!!\n";
    PARAMETERs.reset(nullptr);
  }

  xmlCleanupParser();

  return PARAMETERs.release();
}

Parser::~Parser() {}

}  // namespace EOEPCA
