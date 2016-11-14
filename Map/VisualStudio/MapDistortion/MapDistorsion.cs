﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Xml;
using System.Xml.Serialization;
using System.IO;
using System.Globalization;

namespace MapDistortion
{
    class MapDistorsion
    {
        static int n = 8;
        static int m = 8;
        static int w0;
        static int h0;
        static int w;
        static int h;

        static double[][] A;
        static double[][] B;
        static double[][] C;
        static double[][] D;

        static void Main(string[] args)
        {
            Bitmap image = new Bitmap("DepartementsGabon2.png");

            w0 = image.Width;
            h0 = image.Height;
            w = w0 / n;
            h = h0 / m;

            int dw = w / 10;
            int dh = h / 10;

            A = new double[n + 2][];
            B = new double[n + 2][];
            C = new double[n + 2][];
            D = new double[n + 2][];

            int i, j;
            for (i = 0; i < n + 2; ++i)
            {
                A[i] = new double[m + 2];
                B[i] = new double[m + 2];
                C[i] = new double[m + 2];
                D[i] = new double[m + 2];
            }

            double A0 = 0, B0 = 0, C0 = 0, D0 = 0;
            int count = 0;
         
            for (i = 0; i < n; ++i)
            {
                for (j = 0; j < m; ++j)
                {
                    string path = String.Format("{0}_{1}.kml", i, j);
                    kml kml;

                    XmlReader reader;
                    XmlReaderSettings settings = new XmlReaderSettings();

                    try
                    {
                        reader = XmlReader.Create(path, settings);
                    }
                    catch (Exception)
                    {
                        Console.WriteLine("Fail to open file {0}", path);
                        continue;
                    }

                    Console.WriteLine("Reading file " + path + " ...");

                    XmlSerializer serializer = new XmlSerializer(typeof(kml));

                    try
                    {
                        kml = (kml)serializer.Deserialize(reader);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Fail to deserialize content of file {0}", path);
                        Console.WriteLine(e.Message);
                        if (e.InnerException != null) Console.WriteLine(e.InnerException.Message);
                        return;
                    }

                    reader.Close();

                    int xMin = i * w - dw;
                    if (xMin < 0) xMin = 0;
                    int yMin = j * h - dw;
                    if (yMin < 0) yMin = 0;
                    int xMax = (i + 1) * w + dw;
                    if (xMax > w0) xMax = w0;
                    int yMax = (j + 1) * h + dh;
                    if (yMax > h0) yMax = h0;

                    kmlGroundOverlayLatLonBox box = kml.GroundOverlay.LatLonBox;
                    A[i + 1][j + 1] = (box.east - box.west) / (xMax - xMin);
                    B[i + 1][j + 1] = (box.east * xMin - box.west * xMax) / (xMax - xMin);
                    C[i + 1][j + 1] = (box.south - box.north) / (yMax - yMin);
                    D[i + 1][j + 1] = (box.south * yMin - box.north * yMax) / (yMax - yMin);

                    A0 += A[i + 1][j + 1];
                    B0 += B[i + 1][j + 1];
                    C0 += C[i + 1][j + 1];
                    D0 += D[i + 1][j + 1];
                    ++count;
                }
            }

            A0 /= count;
            B0 /= count;
            C0 /= count;
            D0 /= count;

            using (StreamWriter file = new System.IO.StreamWriter("coeff.csv"))
            {
                writeFile(file, A, A0);
                writeFile(file, B, B0);
                writeFile(file, C, C0);
                writeFile(file, D, D0);
            }
        }

        static void writeFile(StreamWriter file, double[][] T, double T0)
        {
            int i, j;
            for (j = 0; j < m + 2; ++j)
            {
                for (i = 0; i < n + 2; ++i)
                {
                    if (T[i][j] != 0) file.Write((T[i][j] - T0).ToString());
                    if (i != n + 1) file.Write(";");
                }
                file.WriteLine();
            }
            file.WriteLine();
        }
    }
}
