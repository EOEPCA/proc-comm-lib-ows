
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

class NamespaceCWL {
  std::string stac_{""};
  std::string wps_{""};

 public:
  NamespaceCWL() = delete;
  NamespaceCWL(const NamespaceCWL&) = default;
  NamespaceCWL(NamespaceCWL&&) = default;

  explicit NamespaceCWL(const TOOLS::Object* obj) {
    if (obj) {
      for (auto& a : obj->getChildren()) {
        if (a->getF() == XMLNS_STAC) {
          this->stac_ = a->getQ();
        }

        if (a->getF() == XMLNS_WPS1) {
          this->wps_ = a->getQ();
        }
      }
    }
  }

  ~NamespaceCWL() = default;

 public:
  const std::string& getStac() const { return stac_; }
  const std::string& getWps() const { return wps_; }
};

const std::string& Parser::getName() const { return name; }

/// Users/rdirienzo/Project/t2pc/src/t2serverwps/kungfu-wps/schemas.opengis.net/wps/1.0.0/wpsDescribeProcess_response.xsd?#117

enum class OBJECT_NODE { INPUT, OUTPUT };

using MAP_PARSER =
    std::map<std::string,
             const std::function<std::unique_ptr<OWS::Param>(xmlNode*)> >;

auto getWithoutSquareBob = [](const std::string& type) -> std::string {
  auto npos = type.find("[]");
  if (npos != std::string::npos) {
    return type.substr(0, npos);
  }
  return type;
};

auto hasNamespace = [](const std::string& val) -> std::string {
  auto npos = val.find(":");
  if (npos != std::string::npos) {
    return val.substr(0, npos);
  }
  return std::string("");
};

using fnccwl = std::unique_ptr<OWS::Param>(const NamespaceCWL*,
                                           const TOOLS::Object*);
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

void getCWLInputDescriptor(const NamespaceCWL* namespaces,
                           const TOOLS::Object* obj,
                           EOEPCA::OWS::Descriptor& descriptor) {
  std::string id, doc, label;

  if (!obj->hasChildren() && !obj->isQlfEmpty()) {
    id = obj->getQ();
  }

  if (id.empty()) {
    if (!obj->isQlfEmpty()) {
      id = obj->getQ();
    }
    if (id.empty()) {
      id = obj->findAndReturnF("id", "", false);
    }
  }

  label = obj->findAndReturnF("label", "", false);
  doc = obj->findAndReturnF("doc", "", false);

  if (label.empty()) label = id;
  if (doc.empty()) doc = id;

  descriptor.setIdentifier(id);
  descriptor.setTitle(label);
  descriptor.setAbstract(doc);
}

std::unique_ptr<OWS::Param> CWLTypeParserSpecialization(
    const NamespaceCWL* namespaces, const TOOLS::Object* obj,
    EOEPCA::OWS::Descriptor& descriptor) {
  std::unique_ptr<OWS::Param> theReturnParam{nullptr};

  if (!namespaces->getStac().empty()) {
    std::string stacCatalog(namespaces->getStac());
    stacCatalog.append(":catalog");

    auto stacCatalogObj = obj->find(stacCatalog, "", false);
    if (stacCatalogObj) {
      // IS A COMPLEX DATA
      // looking for stac:catalog::format
      std::string stacCatalogFormat(namespaces->getStac());
      stacCatalogFormat.append(":format");

      auto stacFormat = stacCatalogObj->findAndReturnF(stacCatalogFormat, "");
      if (stacFormat.empty()) {
        stacFormat = CWL_STACF_ORMAT_DEFAULT;
      }

      auto cd = std::make_unique<OWS::ComplexData>();
      auto format = std::make_unique<OWS::Format>();
      auto defaultFormat = std::make_unique<OWS::Format>();
      format->setMimeType(stacFormat);
      defaultFormat->setMimeType(stacFormat);
      cd->moveDefaultSupported(defaultFormat);
      cd->moveAddSupported(format);

      cd->setDefaultString(obj->findAndReturnF("default", "", false));

      theReturnParam.reset(cd.release());
    }
  }

  //  if (!theReturnParam) {
  //    // looking for WPS info
  //    std::string wpsNamespace(namespaces->getWps());
  //  }

  if (!theReturnParam) {
    // it is a Literaldata
    auto literData = std::make_unique<OWS::LiteralData>();

    literData->setDataType(descriptor.getTag());
    literData->setDefault(obj->findAndReturnF("default", "", false));

    if (descriptor.getTag() == "enum") {
      literData->setDataType("string");
      auto symbols = obj->find("symbols", "", false);
      if (symbols) {
        for (auto& a : symbols->getChildren()) {
          literData->addAllowedValues(a->getF());
          if (literData->getDefaultValue().empty()) {
            literData->setDataType(a->getF());
          }
        }
      }
    }

    // AllowedValues???
    theReturnParam.reset(literData.release());
  }

  if (theReturnParam) {
    theReturnParam->setTitle(descriptor.getTitle());
    theReturnParam->setAbstract(descriptor.getAbstract());
    theReturnParam->setIdentifier(descriptor.getIdentifier());
    theReturnParam->setMinOccurs(1);
    if (descriptor.isArrayVal()) {
      theReturnParam->setMaxOccurs(999);
    }
  }

  //  std::cout << "descriptor: " << descriptor.getTag()
  //            << " is null:" << (theReturnParam ? "TRUE" : "NULL")
  //            << " ID: " << descriptor.getIdentifier()
  //            << "\n";
  return theReturnParam;
}

/***
 *
 * @param namespaces
 * @param obj
 * @return it type is null or the id is null --> returns null.
 */
std::unique_ptr<OWS::Param> CWLTypeParser(const NamespaceCWL* namespaces,
                                          const TOOLS::Object* obj) {
  //  std::cout << "---------------CWLTypeParser\n";
  //  dumpCWLMODEL(obj,0);

  bool isArray{false};
  std::string type;

  if (!obj->hasChildren() && !obj->isQlfEmpty()) {
    type = obj->getF();
  }
  if (type.empty()) {
    type = obj->findAndReturnF("type", "", false);
  }

  auto a = type.find("[]");
  if (a != std::string::npos) {
    isArray = true;
    type.replace(a, 2, "");
  }

  if (type == "null" || type == "null[]") {
    return nullptr;
  }

  EOEPCA::OWS::Descriptor descriptor;
  descriptor.setTag(type);
  descriptor.setIsArray(isArray);
  getCWLInputDescriptor(namespaces, obj, descriptor);

  if (descriptor.getIdentifier().empty()) {
    return nullptr;
  }

  return CWLTypeParserSpecialization(namespaces, obj, descriptor);
}

void parseCwlInputs(MAP_PARSER_CWL& mapParserCwl,
                    const NamespaceCWL* namespaces, const TOOLS::Object* obj,
                    OWS::OWSProcessDescription* processDescription,
                    std::string_view type) {
  // ptrToNull
  auto fnc = mapParserCwl[""];

  if (!obj->hasChildren() && !obj->isQlfEmpty()) {
    fnc = mapParserCwl[getWithoutSquareBob(obj->getF())];

    if (!fnc) {
      std::string err("type: ");
      err.append(obj->getF()).append(" not supported");
      throw std::runtime_error(err);
    }
  } else {
    auto theType = obj->find("type", "", false);
    if (theType) {
      if (theType->hasChildren()) {
        throw std::runtime_error(
            "The ArraySchema type is supported only his simplification '[]'");
      }
      if (theType->getF() == "record") {
        throw std::runtime_error("Type RecordSchema not supported yet");
      }

      fnc = mapParserCwl[getWithoutSquareBob(theType->getF())];
      if (!fnc) {
        std::string err("type: ");
        err.append(obj->getF()).append(" not supported");
        throw std::runtime_error(err);
      }
    }
  }

  if (fnc) {
    std::unique_ptr<OWS::Param> ptrParam(nullptr);
    ptrParam = fnc(namespaces, obj);
    if (type == "inputs")
      processDescription->addMoveInput(ptrParam);
    else if (type == "outputs")
      processDescription->addMoveOutput(ptrParam);
  }
}

std::unique_ptr<OWS::Param> CWLTypeEnum(const NamespaceCWL* namespaces,
                                        const TOOLS::Object* obj) {
  bool isArray{false};
  std::string type;

  if (!obj->hasChildren() && !obj->isQlfEmpty()) {
    type = obj->getF();
  }
  if (type.empty()) {
    type = obj->findAndReturnF("type", "", false);
  }

  auto a = type.find("[]");
  if (a != std::string::npos) {
    isArray = true;
    type.replace(a, 2, "");
  }

  if (type == "null" || type == "null[]") {
    return nullptr;
  }

  EOEPCA::OWS::Descriptor descriptor;
  descriptor.setTag(type);
  descriptor.setIsArray(isArray);
  getCWLInputDescriptor(namespaces, obj, descriptor);

  if (descriptor.getIdentifier().empty()) {
    descriptor.setIdentifier(obj->findAndReturnF("name", "", false));
  }

  if (descriptor.getIdentifier().empty()) {
    return nullptr;
  }

  return CWLTypeParserSpecialization(namespaces, obj, descriptor);

  return nullptr;
}

void parserOfferingCWL(std::unique_ptr<OWS::OWSOffering>& ptrOffering) {
  if (ptrOffering) {
    MAP_PARSER_CWL mapParser{};
    for (auto& s : CWLTYPE_LIST) mapParser.emplace(FNCMAPS(s, CWLTypeParser));

    mapParser.emplace(FNCMAPS("enum", CWLTypeEnum));

    for (auto& content : ptrOffering->getContents()) {
      if (!content.tag.empty()) {
        auto yamlCwl = std::make_unique<CWL::PARSER::YamlToObject>(
            content.tag.c_str(), content.tag.size());
        auto cwl = yamlCwl->parse();

        auto namespaceCWL =
            std::make_unique<NamespaceCWL>(cwl->find("$namespaces", ""));

        //        dumpCWLMODEL(namespaces,0);
        auto pWorkflow = cwl->find("class", "Workflow");
        if (pWorkflow) {
          auto processDescription =
              std::make_unique<OWS::OWSProcessDescription>();

          processDescription->setIdentifier(
              pWorkflow->findAndReturnF("id", "", false));
          processDescription->setTitle(
              pWorkflow->findAndReturnF("label", "", false));
          processDescription->setAbstract(
              pWorkflow->findAndReturnF("doc", "", false));
          if (processDescription->getTitle().empty()) {
            processDescription->setTitle("NotDefined");
          }

          if (processDescription->getAbstract().empty()) {
            if (!processDescription->getTitle().empty()) {
              processDescription->setAbstract(processDescription->getTitle());
            } else {
              processDescription->setAbstract("NotDefined");
            }
          }

          std::list<const TOOLS::Object*> toParse{
              pWorkflow->find("inputs", ""), pWorkflow->find("outputs", "")};

          for (auto& p : toParse) {
            if (p) {
              if (p->hasChildren()) {
                for (auto& obj : p->getChildren()) {
                  parseCwlInputs(mapParser, namespaceCWL.get(), obj.get(),
                                 processDescription.get(), p->getQ());
                }
              }
            }
          }

          ptrOffering->moveAddProcessDescription(processDescription);

        } else {
          // NO WORKFLOW!!
        }
      }
    }
  }
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
