﻿using MongoDB.Bson;
using MongoDB.Driver;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MapDataProcessing
{
    class DatabaseMapItem
    {
        private readonly ItemId _itemId = new ItemId();
        private readonly Dictionary<XmlResolution, List<GeoPoint>> _lineDictionary = new Dictionary<XmlResolution,List<GeoPoint>>();

        internal void addLine(XmlResolution resolution, List<GeoPoint> line)
        {
            _lineDictionary.Add(resolution, line);
        }

        internal BsonValue Id { get; set; }

        internal List<GeoPoint> getPointList(XmlResolution resolution)
        {
            List<GeoPoint> list;
            if (_lineDictionary.TryGetValue(resolution, out list)) return list;
            return null;
        }

        internal int fillDataBase(IMongoDatabase database, MapData mapData, string itemName)
        {
            IMongoCollection<BsonDocument> pointListCollection = database.GetCollection<BsonDocument>("point_lists");

            BsonArray lineArray = new BsonArray();

            foreach (KeyValuePair<XmlResolution, List<GeoPoint>> pair in _lineDictionary)
            {
                List<GeoPoint> list = pair.Value;
                double resolution = pair.Key.sampleLength1 * Double.Parse(pair.Key.sampleRatio);

                BsonArray pointArray = new BsonArray();
                foreach (GeoPoint point in list)
                {
                    pointArray.Add(point.getBsonDocument(mapData.XmlMapData.parameters.projection));
                }

                BsonDocument pointListDocument = new BsonDocument()
                {
                    { "map", mapData.XmlMapData.parameters.mapId },
                    { "item", itemName },
                    { "resolution", resolution },
                    { "count", list.Count },
                    { "points", pointArray }
                };

                pointListCollection.InsertOne(pointListDocument);

                lineArray.Add(pointListDocument.GetValue("_id"));
            }

            IMongoCollection<BsonDocument> itemCollection = database.GetCollection<BsonDocument>("items");

            BsonDocument itemDocument = new BsonDocument()
            {
                { "item_id", _itemId.Value },
                { "map", mapData.XmlMapData.parameters.mapId },
                { "item", itemName },
                { "point_lists", lineArray }
            };

            itemCollection.InsertOne(itemDocument);

            Id = itemDocument.GetValue("_id");

            return 0;
        }
    }
}
