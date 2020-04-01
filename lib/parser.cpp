
#include "includes/parser.hpp"

#include <iostream>

#include "includes/macro.hpp"

#define echo std::cout

namespace EOEPCA {

const std::string& Parser::getName() const { return name; }



///Users/rdirienzo/Project/t2pc/src/t2serverwps/kungfu-wps/schemas.opengis.net/wps/1.0.0/wpsDescribeProcess_response.xsd?#117

void parseBoundingBoxData(xmlNode* nodeBoundingBoxData) {
  FOR(box, nodeBoundingBoxData) {
    IF_XML_COMPARE(box->name, "Supported") {
      FOR(crs, box) {
        IF_XML_COMPARE(crs->name, "CRS") {
          echo << "\t\t\tSupported: " << (char*)xmlNodeGetContent(crs) << "\n";
        }
      }
    }
    else IF_XML_COMPARE(box->name, "Default") {
      if (box->children &&
          !xmlStrcmp(box->children->name, (const xmlChar*)"CRS")) {
        echo << "\t\t\tDefault: " << (char*)xmlNodeGetContent(box->children)
             << "\n";
      }
    }
  }
}

void parseLiteralData(xmlNode* nodeLiteralData) {
  FOR(ldata, nodeLiteralData) {
    echo << "\t\t" << ldata->name << "\n";
    IS_CHECK(ldata, "AnyValue", XMLNS_WPS1) {}
    else IS_CHECK(ldata, "AllowedValues", XMLNS_OWS) {
      FOR(value, ldata) {
        IS_CHECK(value, "Value", XMLNS_OWS) {
          echo << "\t\t\tValue: " << (char*)xmlNodeGetContent(value) << "\n";
        }
      }
    }
    else IS_CHECK(ldata, "DataType", XMLNS_OWS) {
      echo << "\t\t\tDataType: " << (char*)xmlNodeGetContent(ldata) << "\n";
    }
    else IS_CHECK(ldata, "DefaultValue", XMLNS_OWS) {
      echo << "\t\t\tDefaultValue: " << (char*)xmlNodeGetContent(ldata) << "\n";
    }
  }
}

void parseInput(xmlNode* nodeInput) {
  echo << "*******************\n";
  xmlChar* minOccurs = xmlGetProp(nodeInput, (const xmlChar*)"minOccurs");
  xmlChar* maxOccurs = xmlGetProp(nodeInput, (const xmlChar*)"maxOccurs");
  if (minOccurs) {
    echo << "\t"
         << "minOccurs: " << minOccurs << "\n";
  }
  if (maxOccurs) {
    echo << "\t"
         << "maxOccurs: " << minOccurs << "\n";
  }

  FOR(input, nodeInput) {
    IF_XML_COMPARE(input->ns->href, XMLNS_OWS) {
      IS_CHECK(input, "Identifier", XMLNS_OWS) {
        echo << "\t"
             << "Identifier: " << (char*)xmlNodeGetContent(input) << "\n";
      }
      else IS_CHECK(input, "Title", XMLNS_OWS) {
        echo << "\t"
             << "Title: " << (char*)xmlNodeGetContent(input) << "\n";
      }
      else IS_CHECK(input, "Abstract", XMLNS_OWS) {
        echo << "\t"
             << "Abstract: " << (char*)xmlNodeGetContent(input) << "\n";
      }
    }

    IF_XML_COMPARE(input->ns->href, XMLNS_WPS1) {
      IS_CHECK(input, "LiteralData", XMLNS_WPS1) { parseLiteralData(input); }
      else IS_CHECK(input, "BoundingBoxDatas", XMLNS_WPS1) {
        parseBoundingBoxData(input);
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
  echo << "*******************\n\n\n";
}

void parseOutput(xmlNode* nodeOutput) {}

void parseProcessDescription(xmlNode* processDescription) {
  xmlChar* processVersion =
      xmlGetProp(processDescription, (const xmlChar*)"processVersion");
  if (processVersion) {
    echo << "processVersion: " << (char*)processVersion << "\n";
  }

  FOR(inner_cur_node, processDescription) {
    IS_CHECK(inner_cur_node, "Identifier", XMLNS_OWS) {
      echo << "Identifier: " << (char*)xmlNodeGetContent(inner_cur_node)
           << "\n";
    }
    else IS_CHECK(inner_cur_node, "Title", XMLNS_OWS) {
      echo << "Title: " << (char*)xmlNodeGetContent(inner_cur_node) << "\n";
    }
    else IS_CHECK(inner_cur_node, "Abstract", XMLNS_OWS) {
      echo << "Abstract: " << (char*)xmlNodeGetContent(inner_cur_node) << "\n";
    }
    else IS_CHECK(inner_cur_node, "DataInputs", XMLNS_ATOM) {
      echo << "DATAINPUT*****\n";
      FOR(input, inner_cur_node) { parseInput(input); }
    }
    else IS_CHECK(inner_cur_node, "ProcessOutputs", XMLNS_ATOM) {
      echo << "ProcessOutputs*****\n";
    }
  }
}

void parseOffering(xmlNode* offering_node) {
  FOR(inner_cur_node, offering_node) {
    IS_CHECK(inner_cur_node, "content", XMLNS_OWC) {
      xmlChar* code = xmlGetProp(inner_cur_node, (const xmlChar*)"type");
      xmlChar* href = xmlGetProp(inner_cur_node, (const xmlChar*)"href");
      if (code && href) {
        echo << "IS_CONTENT TYPE: " << (char*)code << " href: " << (char*)href
             << "\n";
      }
    }
    else IS_CHECK(inner_cur_node, "operation", XMLNS_OWC) {
      xmlChar* code = xmlGetProp(inner_cur_node, (const xmlChar*)"code");
      if (code) {
        echo << "code: " << (char*)code << "\n";
      }

      IF_XML_COMPARE(code, "DescribeProcess") {
        echo << "IF_XML_COMPARE IS_OPERATION TYPE: " << (char*)code << "\n";
        if (inner_cur_node->children) {
          FOR(desc, inner_cur_node->children) {
            IS_CHECK(desc, "ProcessDescription", XMLNS_ATOM) {
              parseProcessDescription(desc);
            }
          }
        }
      }
    }
  }
}

void parseEntry(xmlNode* entry_node) {
  FOR(inner_cur_node, entry_node) {
    if (inner_cur_node->type == XML_COMMENT_NODE) {
      continue;
    } else if (inner_cur_node->type == XML_ELEMENT_NODE) {
      IS_CHECK(inner_cur_node, "identifier", XMLNS_PURL) {
        echo << (char*)xmlNodeGetContent(inner_cur_node) << "\n";
      }
      else IS_CHECK(inner_cur_node, "offering", XMLNS_OWC) {
        parseOffering(inner_cur_node);
      }
    }
  }
}

int Parser::parseXml(const char* bufferXml, int size) {
  int ret = 0;
  xmlDoc* doc = nullptr;
  xmlNode* root_element = nullptr;
  LIBXML_TEST_VERSION
  int option = XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS |
               XML_PARSE_NOBLANKS;

  std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> pDoc{
      xmlReadMemory(bufferXml, size, nullptr, nullptr, option), &xmlFreeDoc};

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
                  parseEntry(inner_entry_node);
                }
              }
            }
          }
        }
      }
    }

  } catch (std::runtime_error& err) {
    std::cerr << err.what() << "\n";
    ret = 3;
  } catch (...) {
    std::cerr << "CATCH!!!\n";
    ret = 3;
  }

  xmlCleanupParser();

  return ret;
}

Parser::~Parser() {}

}  // namespace EOEPCA
