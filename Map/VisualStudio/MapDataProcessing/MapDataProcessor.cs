﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Schema;
using System.Xml.Serialization;
using System.IO;
using MongoDB.Driver;
using MongoDB.Bson;

namespace MapDataProcessing
{
    class MapDataProcessor
    {
        private readonly String _configFile;
        private MapData _mapData;
        private readonly Dictionary<String, MapElement> _elementDictionary = new Dictionary<string, MapElement>();
        private readonly Dictionary<String, int> _fileNameDictionary = new Dictionary<string, int>();

        internal MapDataProcessor(String configFile)
        {
            _configFile = configFile;
        }

        internal int run()
        {
            int result = 0;

            if (result == 0) result = loadData();
            if (result == 0) result = createElements();
            if (result == 0) result = addKmlFiles(_mapData.XmlMapData.parameters.kmlDir);
            if (result == 0) result = formParts();
            if (result == 0) result = PolygonLinePart.smoothAll(_mapData);
            if (result == 0) result = PolygonPolygonPart.smoothAll(_mapData);
            if (result == 0) result = formContours();
            if (result == 0) result = fillDatabase();

            return result;
        }

        private int loadData()
        {
            XmlReader reader;
            XmlReaderSettings settings = new XmlReaderSettings();
            settings.ValidationType = ValidationType.Schema;
            settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessInlineSchema;
            settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessSchemaLocation;
            settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessIdentityConstraints;

            try
            {
                reader = XmlReader.Create(_configFile, settings);
            }
            catch (Exception e)
            {
                Console.WriteLine("Fail to open file {0}", _configFile);
                Console.WriteLine(e.Message);
                return -1;
            }

            Console.WriteLine("Reading configuration file " + _configFile + " ...");

            XmlSerializer serializer = new XmlSerializer(typeof(XmlMapData));
            XmlMapData mapData = null;

            try
            {
                mapData = (XmlMapData)serializer.Deserialize(reader);
            }
            catch (Exception e)
            {
                Console.WriteLine("Fail to deserialize content of file {0}", _configFile);
                Console.WriteLine(e.Message);
                if (e.InnerException != null) Console.WriteLine(e.InnerException.Message);
                return -1;
            }

            reader.Close();

            _mapData = new MapData(_configFile, mapData);

            MessageLogger.LogFileName = mapData.parameters.logFileName;
            MessageLogger.FileLogLevel = mapData.parameters.fileLogLevel;
            MessageLogger.ConsoleLogLevel = mapData.parameters.consoleLogLevel;

            return 0;
        }

        private int createElements()
        {
            foreach (XmlPolygonElement xmlPolygonElement in _mapData.XmlMapData.elementList.polygonElementList)
            {
                String id = xmlPolygonElement.id.Substring(2);
                PolygonMapElement polygonMapElement = new PolygonMapElement(id, _mapData);
                _elementDictionary.Add(id, polygonMapElement);
            }

            foreach (XmlLineElement xmlLineElement in _mapData.XmlMapData.elementList.lineElementList)
            {
                String id = xmlLineElement.id.Substring(2);
                LineMapElement lineMapElement = new LineMapElement(id, _mapData);
                _elementDictionary.Add(id, lineMapElement);
            }

            foreach (XmlPointElement xmlPointElement in _mapData.XmlMapData.elementList.pointElementList)
            {
                String id = xmlPointElement.id.Substring(2);
                PointMapElement pointMapElement = new PointMapElement(id, _mapData);
                _elementDictionary.Add(id, pointMapElement);
            }

            return 0;
        }

        private int addKmlFiles(String path)
        {
            FileAttributes attr;
            try
            {
                attr = File.GetAttributes(path);
            }
            catch (FileNotFoundException)
            {
                MessageLogger.addMessage(XmlLogLevelEnum.ERROR, String.Format("Kml path {0} not found", path));
                return -1;
            }

            if ((attr & FileAttributes.Directory) == FileAttributes.Directory)
            {
                MessageLogger.addMessage(XmlLogLevelEnum.MESSAGE, String.Format("Browsing Kml path {0} ...", path));
                List<String> entries = new List<String>(Directory.EnumerateFileSystemEntries(path));
                foreach (String entry in entries)
                {                
                    if (addKmlFiles(entry) != 0) return -1;
                }
            }
            else
            {
                if (String.Equals(Path.GetExtension(path), ".kml", StringComparison.OrdinalIgnoreCase))
                {
                    if (addKmlFile(path) != 0) return -1;
                }
            }

            return 0;
        }

        private int addKmlFile(String path)
        {
            String name = Path.GetFileNameWithoutExtension(path);
            if (_fileNameDictionary.ContainsKey(name))
            {
                MessageLogger.addMessage(XmlLogLevelEnum.ERROR, String.Format("Several KMl files have the same name '{0}'", name));
                return -1;
            }

            _fileNameDictionary.Add(name, 0);
            String[] idList = name.Split('_');
            foreach (String id in idList)
            {
                MapElement element;
                if (_elementDictionary.TryGetValue(id, out element))
                {
                    if (element.addKmlFile(path) != 0) return -1;
                }
            }

            return 0;
        }

        private int formParts()
        {
            foreach (MapElement element in _elementDictionary.Values)
            {
                if (element.formParts() != 0) return -1;
            }

            return 0;
        }

        private int formContours()
        {
            foreach (PolygonMapElement element in _elementDictionary.Values.OfType<PolygonMapElement>())
            {
                if (element.formContours() != 0) return -1;
            }

            return 0;
        }

        private int fillDatabase()
        {
            MongoClient mongoClient = new MongoClient();
            IMongoDatabase database = mongoClient.GetDatabase(_mapData.XmlMapData.parameters.databaseName);

            FilterDefinition<BsonDocument> filter = Builders<BsonDocument>.Filter.Eq("map", _mapData.XmlMapData.parameters.mapId);
            IMongoCollection<BsonDocument> levelCollection = database.GetCollection<BsonDocument>("point_lists");
            levelCollection.DeleteMany(filter);

            if (PolygonLinePart.fillDatabase(database, _mapData) != 0) return -1;
            if (PolygonPolygonPart.fillDatabase(database, _mapData) != 0) return -1;

            return 0;
        }
    }
}