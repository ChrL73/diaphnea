﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace QuestionInstantiation
{
    class Level
    {
        private XmlLevel _xmlLevel;
        private Int32 _value;
        private readonly Dictionary<string, Element> _elementDictionary = new Dictionary<string, Element>();
        private readonly Dictionary<XmlElementType, List<Element>> _elementByTypeDictionary = new Dictionary<XmlElementType, List<Element>>();
        private readonly List<Category> _categoryList = new List<Category>();

        internal int initialize(QuizData quizData, XmlLevel xmlLevel)
        {
            _xmlLevel = xmlLevel;
            _value = Int32.Parse(_xmlLevel.value);

            if (addElements(quizData) != 0) return -1;

            return 0;
        }

        private int addElements(QuizData quizData)
        {
            Dictionary<string, List<Element>> elementByAttributeKeyDictionary = new Dictionary<string, List<Element>>();

            foreach (XmlElement xmlElement in quizData.XmlQuizData.elementList)
	        {
                Int32 minLevel = Int32.Parse(xmlElement.minLevel);
                if (_value >= minLevel)
		        {
                    XmlElementType xmlElementType = quizData.getXmlElementType(xmlElement.type);
                    Element element = new Element(xmlElement, xmlElementType);

                    if (element.addAttributes(quizData) != 0) return -1;
                    if (element.addNumericalAttributes(quizData) != 0) return -1;

                    _elementDictionary.Add(xmlElement.id, element);

			        foreach (string attributeKey in element.AttributeKeyList)
			        {
                        if (!elementByAttributeKeyDictionary.ContainsKey(attributeKey)) elementByAttributeKeyDictionary.Add(attributeKey, new List<Element>());
                        elementByAttributeKeyDictionary[attributeKey].Add(element);
			        }
		        }
	        }

	        foreach (KeyValuePair<string, List<Element>> pair in elementByAttributeKeyDictionary)
	        {
                List<Element> elementList = pair.Value;

                if (elementList.Count > 1)
		        {
                    string key = pair.Key;
                    string eltTypeStr = key.Substring(0, key.IndexOf('+'));
                    string attrTypeStr = key.Substring(key.IndexOf('+') + 1);
                    string valueStr = attrTypeStr.Substring(attrTypeStr.IndexOf('+') + 1);
                    attrTypeStr = attrTypeStr.Substring(0, attrTypeStr.IndexOf('+')); 

                    foreach (Element element in elementList)
			        {			
                        MessageLogger.addMessage(XmlLogLevelEnum.WARNING, String.Format(
                            "Level \"{0}\": Element {1} has the same value ({2}) for attribute {3} as another element of same type ({4}). Since the attribute type {3} can be used as a question, the element {1} is ignored to avoid ambiguous questions",
                            _xmlLevel.name, element.XmlElement.id, valueStr, attrTypeStr, eltTypeStr));

                        _elementDictionary.Remove(element.XmlElement.id);
			        }
		        }
	        }

            foreach (KeyValuePair<string, Element> pair in _elementDictionary)
	        {
                Element element = pair.Value;

                if (element.addRelations(quizData, _elementDictionary) == 0)
		        {
                    if (!_elementByTypeDictionary.ContainsKey(element.Type)) _elementByTypeDictionary.Add(element.Type, new List<Element>());
                    _elementByTypeDictionary[element.Type].Add(element);
		        }
		        else
		        {
			        return -1;
		        }
	        }

            return 0;
        }
    }
}
