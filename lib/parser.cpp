
#include "includes/parser.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>

#include "includes/httpfuntions.hpp"
#include "includes/macro.hpp"
#include "includes/yaml/yaml.hpp"
//#define echo std::cout <<

namespace EOEPCA {

const std::string& Parser::getName() const { return name; }

/// Users/rdirienzo/Project/t2pc/src/t2serverwps/kungfu-wps/schemas.opengis.net/wps/1.0.0/wpsDescribeProcess_response.xsd?#117

enum class OBJECT_NODE { INPUT, OUTPUT };

using MAP_PARSER =
    std::map<std::string,
             const std::function<std::unique_ptr<OWS::Param>(xmlNode*)> >;

using fnccwl = std::unique_ptr<OWS::Param>(const TOOLS::Object*);
using tFnccwl = const std::function<fnccwl>;
using MAP_PARSER_CWL = std::map<std::string, EOEPCA::tFnccwl>;

std::unique_ptr<OWS::Format> getFormat(xmlNode* nodeComplexDataFormat) {
  auto format = std::make_unique<OWS::Format>();

  FOR(f, nodeComplexDataFormat) {
    if (XML_COMPARE(f->name, "MimeType")) {
      format->setMimeType(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    } else if (XML_COMPARE(f->name, "Encoding")) {
      format->setEncoding(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    } else if (XML_COMPARE(f->name, "Schema")) {
      format->setSchema(std::string(CHAR_BAD_CAST xmlNodeGetContent(f)));
    }
  }

  return format;
}

[[nodiscard]] std::unique_ptr<OWS::Param> parseComplexData(
    xmlNode* nodeComplexData) {
  // echo "PARSECOMPLEXDATA DATA\n";

  auto complexData = std::make_unique<OWS::ComplexData>();

  xmlChar* maximumMegabytes =
      xmlGetProp(nodeComplexData, (const xmlChar*)"maximumMegabytes");

  if (maximumMegabytes) {
    complexData->setMaximumMegabytes(
        std::string(CHAR_BAD_CAST maximumMegabytes));
  }

  FOR(box, nodeComplexData) {
    if (XML_COMPARE(box->name, "Default")) {
      FOR(format, box) {
        auto mFrmat = getFormat(format);
        complexData->moveDefaultSupported(mFrmat);
      }
    } else if (XML_COMPARE(box->name, "Supported")) {
      FOR(format, box) {
        auto mFrmat = getFormat(format);
        complexData->moveAddSupported(mFrmat);
      }
    }
  }

  return std::move(complexData);
}

[[nodiscard]] std::unique_ptr<OWS::Param> parseBoundingBoxData(
    xmlNode* nodeBoundingBoxData) {
  // echo "PARSEBOUNDINGBOXDATA DATA\n";

  auto boundingBoxData = std::make_unique<OWS::BoundingBoxData>();

  FOR(box, nodeBoundingBoxData) {
    if (XML_COMPARE(box->name, "Supported")) {
      FOR(crs, box) {
        if (XML_COMPARE(crs->name, "CRS")) {
          boundingBoxData->addSupportedValues(
              std::string(CHAR_BAD_CAST xmlNodeGetContent(crs)));
        }
      }
    } else if (XML_COMPARE(box->name, "Default")) {
      if (box->children &&
          !xmlStrcmp(box->children->name, (const xmlChar*)"CRS")) {
        boundingBoxData->setDefault(
            std::string(CHAR_BAD_CAST xmlNodeGetContent(box->children)));
      }
    }
  }

  return std::move(boundingBoxData);
}

[[nodiscard]] std::unique_ptr<OWS::Param> parseLiteralData(
    xmlNode* nodeLiteralData) {
  auto literData = std::make_unique<OWS::LiteralData>();

  // echo "LITERLA DATA\n";

  FOR(ldata, nodeLiteralData) {
    if (IS_CHECK(ldata, "AnyValue", XMLNS_WPS1)) {
    } else if (IS_CHECK(ldata, "AllowedValues", XMLNS_OWS)) {
      FOR(value, ldata) {
        if (IS_CHECK(value, "Value", XMLNS_OWS)) {
          literData->addAllowedValues(
              std::string(CHAR_BAD_CAST xmlNodeGetContent(value)));
        }
      }
    } else if (IS_CHECK(ldata, "DataType", XMLNS_OWS)) {
      literData->setDataType(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(ldata)));
    } else if (IS_CHECK(ldata, "DefaultValue", XMLNS_OWS)) {
      literData->setDefault(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(ldata)));
    }
  }

  return std::move(literData);
}

void parseObject(MAP_PARSER& mapParser, xmlNode* nodeObject,
                 OBJECT_NODE objectNode,
                 std::unique_ptr<OWS::OWSProcessDescription>& ptrParams) {
  std::unique_ptr<OWS::Param> param{nullptr};

  auto ptrOccurs = std::make_unique<OWS::Occurs>();
  auto ptrDescriptor = std::make_unique<OWS::Descriptor>();

  xmlChar* minOccurs = xmlGetProp(nodeObject, (const xmlChar*)"minOccurs");
  xmlChar* maxOccurs = xmlGetProp(nodeObject, (const xmlChar*)"maxOccurs");
  if (minOccurs) {
    ptrOccurs->setMinOccurs(std::string(CHAR_BAD_CAST minOccurs));
  }
  if (maxOccurs) {
    ptrOccurs->setMaxOccurs(std::string(CHAR_BAD_CAST maxOccurs));
  }

  FOR(input, nodeObject) {
    if (IS_CHECK(input, "Identifier", XMLNS_OWS)) {
      ptrDescriptor->setIdentifier(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
    } else if (IS_CHECK(input, "Title", XMLNS_OWS)) {
      ptrDescriptor->setTitle(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
    } else if (IS_CHECK(input, "Abstract", XMLNS_OWS)) {
      ptrDescriptor->setAbstract(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(input)));
    }

    if (auto f = mapParser[std::string(CHAR_BAD_CAST input->name)]) {
      if (f) param = f(input);
    }
  }

  if (param) {
    *param << *ptrDescriptor << *ptrOccurs;
    switch (objectNode) {
      case OBJECT_NODE::INPUT: {
        ptrParams->addMoveInput(param);
        break;
      }
      case OBJECT_NODE::OUTPUT: {
        ptrParams->addMoveOutput(param);
        break;
      }
    }
  }
}

void parseOutput(xmlNode* nodeOutput) {}

void parseProcessDescription(
    xmlNode* processDescription,
    std::unique_ptr<OWS::OWSProcessDescription>& ptrParams) {
  MAP_PARSER mapParser{};
  mapParser.emplace(FNCMAP(LiteralData, parseLiteralData));
  mapParser.emplace(FNCMAP(LiteralOutput, parseLiteralData));
  mapParser.emplace(FNCMAP(BoundingBoxData, parseBoundingBoxData));
  mapParser.emplace(FNCMAP(BoundingBoxOutput, parseBoundingBoxData));
  mapParser.emplace(FNCMAP(ComplexData, parseComplexData));
  mapParser.emplace(FNCMAP(ComplexOutput, parseComplexData));

  xmlChar* processVersion =
      xmlGetProp(processDescription, (const xmlChar*)"processVersion");
  if (processVersion) {
    ptrParams->setVersion(std::string(CHAR_BAD_CAST processVersion));
  }

  FOR(inner_cur_node, processDescription) {
    if (IS_CHECK(inner_cur_node, "Identifier", XMLNS_OWS)) {
      ptrParams->setIdentifier(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    } else if (IS_CHECK(inner_cur_node, "Title", XMLNS_OWS)) {
      ptrParams->setTitle(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    } else if (IS_CHECK(inner_cur_node, "Abstract", XMLNS_OWS)) {
      ptrParams->setAbstract(
          std::string(CHAR_BAD_CAST xmlNodeGetContent(inner_cur_node)));
    } else if (IS_CHECK(inner_cur_node, "DataInputs", XMLNS_ATOM)) {
      FOR(input, inner_cur_node) {
        // parser inputs
        parseObject(mapParser, input, OBJECT_NODE::INPUT, ptrParams);
      }
    } else if (IS_CHECK(inner_cur_node, "ProcessOutputs", XMLNS_ATOM)) {
      // parser outputs
      FOR(I, inner_cur_node) {
        if (I->children && XML_COMPARE(I->name, "Output")) {
          parseObject(mapParser, I, OBJECT_NODE::OUTPUT, ptrParams);
        }
      }
    }
  }
}

void dumpCWLMODEL(const TOOLS::Object* m, int level) {
  level++;
  std::string tab = std::string((level * 2), ' ');
  std::cout << tab << "id: \"" << m->getQ() << "\" val: \"" << m->getF() << "\""
            << "\n";

  for (auto& i : m->getChildren()) {
    dumpCWLMODEL(i.get(), level);
  }
}

std::unique_ptr<OWS::Param> Enum(const TOOLS::Object* S) { return nullptr; }

std::unique_ptr<OWS::Param> Simple(const TOOLS::Object* S) {
  std::cout << "---------------SIMPLE\n";
  //  dumpCWLMODEL(S, 0);



  return nullptr;
}

std::unique_ptr<OWS::Param> parseCwlInputsArray(const TOOLS::Object* S) {
  std::cout << "---------------SimpleArray\n";
  dumpCWLMODEL(S, 0);

  return nullptr;
}

void parseCwlInputs(MAP_PARSER_CWL& mapParserCwl, const TOOLS::Object* obj,
                    std::unique_ptr<OWS::OWSOffering>& ptrOffering) {
  if (!obj->hasChildren() && !obj->isQlfEmpty()) {
    if (auto f = mapParserCwl[obj->getF()]) {
      if (f) f(obj);
    }
  } else {
    auto theType = obj->find("type", "", false);
    if (theType) {
      if (auto f = mapParserCwl[theType->getF()]) {
        if (f) f(obj);
      }
    }
  }
}
void parseCwlOutputs(MAP_PARSER_CWL& mapParserCwl, const TOOLS::Object* obj,
                     std::unique_ptr<OWS::OWSOffering>& ptrOffering) {}

void parserOfferingCWL(std::unique_ptr<OWS::OWSOffering>& ptrOffering) {
  if (ptrOffering) {
    MAP_PARSER_CWL mapParser{};
    for (auto& s : CWLTYPE_LIST) mapParser.emplace(FNCMAPS(s, Simple));
    for (auto& s : CWLTYPE_LIST_ARRAY)
      mapParser.emplace(FNCMAPS(s, parseCwlInputsArray));
    mapParser.emplace(FNCMAPS("enum", Enum));

    for (auto& content : ptrOffering->getContents()) {
      if (!content.tag.empty()) {
        auto yamlCwl = std::make_unique<CWL::PARSER::YamlToObject>(
            content.tag.c_str(), content.tag.size());
        auto cwl = yamlCwl->parse();

        auto pWorkflow = cwl->find("class", "Workflow");
        if (pWorkflow) {
          auto pInputs = pWorkflow->find("inputs", "");
          if (pInputs) {
            if (pInputs->hasChildren()) {
              for (auto& obj : pInputs->getChildren()) {
                parseCwlInputs(mapParser, obj.get(), ptrOffering);
              }
            }
          }

          auto pOutputs = pWorkflow->find("outputs", "");
          if (pOutputs) {
            if (pOutputs->hasChildren()) {
              for (auto& obj : pOutputs->getChildren()) {
                parseCwlOutputs(mapParser, obj.get(), ptrOffering);
              }
            }
          }

        } else {
          // NO WORKFLOW!!
        }
      }
    }
  }

  //  if (content.type == XML_CWL_TYPE) {
  //
  //    std::unique_ptr<TOOLS::Object> father{};
  ////    CWL::PARSER::YamlToObject()
  //
  //
  //
  //  }
}

void parseOffering(xmlNode* offering_node,
                   std::unique_ptr<OWS::OWSOffering>& ptrOffering) {
  xmlChar* code = xmlGetProp(offering_node, (const xmlChar*)"code");
  if (code) {
    ptrOffering->setCode(std::string(CHAR_BAD_CAST code));
  }

  FOR(inner_cur_node, offering_node) {
    if (IS_CHECK(inner_cur_node, "content", XMLNS_OWC)) {
      xmlChar* type = xmlGetProp(inner_cur_node, (const xmlChar*)"type");
      xmlChar* href = xmlGetProp(inner_cur_node, (const xmlChar*)"href");

      ptrOffering->addContent(std::string(CHAR_BAD_CAST type),
                              std::string(CHAR_BAD_CAST href));

    } else if (IS_CHECK(inner_cur_node, "operation", XMLNS_OWC)) {
      xmlChar* code = xmlGetProp(inner_cur_node, (const xmlChar*)"code");
      if (code) {
        if (XML_COMPARE(code, "DescribeProcess")) {
          if (inner_cur_node->children) {
            FOR(desc, inner_cur_node->children) {
              if (IS_CHECK(desc, "ProcessDescription", XMLNS_ATOM)) {
                auto ptrParams = std::make_unique<OWS::OWSProcessDescription>();
                parseProcessDescription(desc, ptrParams);
                ptrOffering->moveAddProcessDescription(ptrParams);
              }
            }
          }
        }
      }
    }
  }
}

void parseEntry(xmlNode* entry_node, std::unique_ptr<OWS::OWSEntry>& owsEntry) {
  FOR(inner_cur_node, entry_node) {
    if (inner_cur_node->type == XML_COMMENT_NODE) {
      continue;
    } else if (inner_cur_node->type == XML_ELEMENT_NODE) {
      if (IS_CHECK(inner_cur_node, "identifier", XMLNS_PURL)) {
        // set /entry/identifier/
        owsEntry->setPackageIdentifier(
            std::string((char*)xmlNodeGetContent(inner_cur_node)));

      } else if (IS_CHECK(inner_cur_node, "offering", XMLNS_OWC)) {
        auto ptrOffering = std::make_unique<OWS::OWSOffering>();

        parseOffering(inner_cur_node, ptrOffering);

        for (auto& content : ptrOffering->getContents()) {
          if (content.type == XML_CWL_TYPE) {
            auto res = getFromWeb(content.tag, content.href.c_str());
            if (res != 200) {
              std::string err{"href: "};
              err.append(content.href).append(" can't be downloaded");
              throw std::runtime_error(err);
            }
          }
        }

        if (ptrOffering->getCode() == OFFERING_CODE_CWL) {
          parserOfferingCWL(ptrOffering);
        }
        owsEntry->moveAddOffering(ptrOffering);
      }
    }
  }
}

OWS::OWSContext* Parser::parseXml(const char* bufferXml, int size) {
  int ret = 0;
  xmlDoc* doc = nullptr;
  xmlNode* root_element = nullptr;
  LIBXML_TEST_VERSION
  int option = XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS |
               XML_PARSE_NOBLANKS;

  std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> pDoc{
      xmlReadMemory(bufferXml, size, nullptr, nullptr, option), &xmlFreeDoc};

  auto owsContext = std::make_unique<OWS::OWSContext>();

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

        if (IS_CHECK(cur_node, "feed", XMLNS_ATOM)) {
          if (cur_node->children != nullptr) {
            FOR(inner_entry_node, cur_node) {
              if (inner_entry_node->type == XML_COMMENT_NODE) {
                continue;
              } else if (inner_entry_node->type == XML_ELEMENT_NODE) {
                if (IS_CHECK(inner_entry_node, "entry", XMLNS_ATOM)) {
                  auto owsEntry = std::make_unique<OWS::OWSEntry>();
                  parseEntry(inner_entry_node, owsEntry);
                  owsContext->moveAddEntry(owsEntry);
                }
              }
            }
          }
        }
      }
    }

  } catch (std::runtime_error& err) {
    std::cerr << err.what() << "\n";
    owsContext.reset(nullptr);
  } catch (...) {
    std::cerr << "CATCH!!!\n";
    owsContext.reset(nullptr);
  }

  xmlCleanupParser();

  return owsContext.release();
}

Parser::~Parser() {}

}  // namespace EOEPCA
