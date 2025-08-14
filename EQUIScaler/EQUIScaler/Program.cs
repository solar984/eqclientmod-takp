using ImageMagick;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;

namespace EQUIScaler
{
    class Program
    {
        public static string defaultUIDir = @"C:\uifiles\default-pristine";
        public static string outputUIDir = @"C:\uifiles\defaultx2";
        //public static string defaultUIDir = @"C:\uifiles\poweroftwo-pristine";
        //public static string outputUIDir = @"C:\uifiles\poweroftwox2";
        //public static string defaultUIDir = @"C:\uifiles\vert-pristine";
        //public static string outputUIDir = @"C:\uifiles\vertx2";
        public static int scale = 2;

        static void Main(string[] args)
        {
            try
            {
                //foreach (var file in Directory.EnumerateFiles(defaultUIDir, "EQUI_*.xml"))
                foreach (var fileName in GetIncludesFromEQUI(Path.Combine(defaultUIDir, "EQUI.xml")))
                {
                    string file = Path.Combine(defaultUIDir, fileName);
                    Console.WriteLine("Processing {0}", fileName);
                    bool changed = new UISizeScaler().ProcessFile(file, Path.Combine(outputUIDir, fileName), scale);
                    if (!changed)
                    {
                        Console.WriteLine("No changes.");
                    }
                    else
                    {
                        Console.WriteLine("Modified {0}", fileName);
                    }
                    Console.WriteLine("Done with {0}", fileName);
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            finally
            {
                Console.WriteLine("Done.");
                Console.Read();
            }
        }

        static string[] GetIncludesFromEQUI(string path)
        {
            List<string> list = new List<string>();
            XmlDocument doc = new XmlDocument();
            doc.Load(path);
            foreach (var node in doc.SelectNodes("/XML/Composite/Include"))
            {
                XmlElement targetelem = node as XmlElement;
                if (targetelem != null)
                {
                    if (targetelem.InnerText != null && targetelem.InnerText.StartsWith("EQUI_"))
                    {
                        list.Add(targetelem.InnerText);
                    }
                }
            }

            return list.ToArray();
        }


    }

    public class UISizeScaler
    {
        public bool ProcessFile(string pathInput, string pathOutput, int scale)
        {
            bool changed = false;
            string fileName = Path.GetFileName(pathInput);

            XmlDocument doc = new XmlDocument();
            doc.Load(pathInput);

            XmlNamespaceManager ns = new XmlNamespaceManager(doc.NameTable);
            //ns.AddNamespace("eq", "EverQuestData");
            ns.AddNamespace("dt", "EverQuestDataTypes");

#if true
            // list of elements comtaining an integer value to be scaled
            var xpathScaleValuesList = new string[]
            {
                "/XML/*/Location/X",
                "/XML/*/Location/Y",
                "/XML/*/DecalOffset/X",
                "/XML/*/DecalOffset/Y",
                "/XML/*/DecalSize/CX",
                "/XML/*/DecalSize/CY",
                "/XML/*/Size/CX",
                "/XML/*/Size/CY",
                "/XML/SpellGem/SpellIconOffsetX", // only in EQUI_CastSpellWnd.xml
                "/XML/SpellGem/SpellIconOffsetY", // only in EQUI_CastSpellWnd.xml
                "/XML/*/LeftAnchorOffset",
                "/XML/*/TopAnchorOffset",
                "/XML/*/RightAnchorOffset",
                "/XML/*/BottomAnchorOffset",
                "/XML/Gauge/GaugeOffsetY",
                "/XML/Listbox/Columns/Width",
            };

            var skipAncestors = new string[]
            {
                "TextureInfo",
                "Ui2DAnimation",
            };

            // apply value scaling to each matching element
            foreach (string xpath in xpathScaleValuesList)
            {
                // compass doesn't work enlarged
                if(doc.SelectSingleNode("/XML/Screen[@item='CompassWindow']") != null) continue;

                foreach (var node in doc.SelectNodes(xpath, ns))
                {
                    XmlElement targetElem = node as XmlElement;
                    if (targetElem != null)
                    {
                        // trace up and find ancestors we don't want to modify - texture sizes and stuff
                        bool skip = false;
                        XmlNode ancestor = targetElem;
                        do
                        {
                            ancestor = ancestor.ParentNode;
                            if (Array.Find(skipAncestors, a => ancestor.Name.Equals(a)) != null)
                            {
                                skip = true;
                            }
                        }
                        while (!skip && ancestor != doc.DocumentElement);
                        if (skip)
                        {
                            continue;
                        }

                        // don't modify position of non relative windows (top level, screen based offset)
                        if (targetElem.ParentNode.Name.Equals("Location"))
                        {
                            XmlElement relposElem = targetElem.ParentNode.ParentNode["RelativePosition"];
                            if (relposElem != null && relposElem.InnerText != null && relposElem.InnerText.Equals("false"))
                            {
                                continue;
                            }
                        }

                        int val;
                        if (Int32.TryParse(targetElem.InnerText, out val))
                        {
                            val *= scale;
                            //Console.WriteLine("{0}:{1}    {2} -> {3}", fileName, GetXPath_SequentialIteration(targetElem), targetElem.InnerXml, val);
                            targetElem.InnerXml = val.ToString(CultureInfo.InvariantCulture);
                            changed = true;
                        }
                    }
                }
            }

            // layout locations
            XmlDocument layoutDoc = new XmlDocument();
            layoutDoc.Load("defaultx2-layout.xml");
            foreach (var node in doc.SelectNodes("/XML/Screen", ns))
            {
                XmlElement targetElem = node as XmlElement;
                if (targetElem != null)
                {
                    string screenName = targetElem.GetAttribute("item").Trim();
                    XmlElement screenLayout = layoutDoc.SelectSingleNode(String.Format("/Screens/Screen[@item='{0}']", screenName)) as XmlElement;
                    if (screenLayout != null)
                    {
                        foreach (XmlNode replacement in screenLayout.ChildNodes)
                        {
                            if (replacement is XmlElement)
                            {
                                foreach (XmlNode target in targetElem.ChildNodes)
                                {
                                    if (target is XmlElement && target.Name.Equals(replacement.Name))
                                    {
                                        target.InnerXml = replacement.InnerXml;
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
#endif

            string[] tgas = Directory.GetFiles(Path.GetDirectoryName(pathInput), "*.tga").Select(s => Path.GetFileName(s)).ToArray();
            List<string> tgasToSize = new List<string>();
            foreach (string tga in tgas)
            {
                // find TextureInfos
                foreach (var pnode in doc.SelectNodes("/XML/TextureInfo", ns))
                {
                    XmlElement tiElem = pnode as XmlElement;
                    if (tiElem != null)
                    {
                        var attr = tiElem.Attributes["item"];
                        if (attr != null && attr.Value != null && attr.Value.Equals(tga))
                        {
                            foreach (var node in tiElem.SelectNodes("Size/CX | Size/CY"))
                            {
                                XmlElement targetElem = node as XmlElement;
                                if (node != null)
                                {
                                    int val;
                                    if (Int32.TryParse(targetElem.InnerText, out val))
                                    {
                                        val *= scale;
                                        //Console.WriteLine("{0}:{1}    {2} -> {3}", fileName, GetXPath_SequentialIteration(targetElem), targetElem.InnerXml, val);
                                        targetElem.InnerXml = val.ToString(CultureInfo.InvariantCulture);
                                        changed = true;
                                        if (!tgasToSize.Contains(tga))
                                        {
                                            tgasToSize.Add(tga);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // find Frames that use this tga
                foreach (var pnode in doc.SelectNodes("/XML/Ui2DAnimation"))
                {
                    XmlElement uiElem = pnode as XmlElement;
                    if (uiElem != null)
                    {
                        XmlElement framesElem = uiElem["Frames"];
                        if (framesElem != null)
                        {
                            var textureElem = framesElem["Texture"];
                            if (textureElem != null && textureElem.InnerText != null && textureElem.InnerText.Equals(tga))
                            {
                                foreach (var node in uiElem.SelectNodes("Frames/Size/CX | Frames/Size/CY | Frames/Location/X | Frames/Location/Y | Frames/Hotspot/X | Frames/Hotspot/Y | CellWidth | CellHeight"))
                                {
                                    XmlElement targetElem = node as XmlElement;
                                    if (targetElem != null)
                                    {
                                        int val;
                                        if (Int32.TryParse(targetElem.InnerText, out val))
                                        {
                                            val *= scale;
                                            //Console.WriteLine("{0}:{1}    {2} -> {3}", fileName, GetXPath_SequentialIteration(targetElem), targetElem.InnerXml, val);
                                            targetElem.InnerXml = val.ToString(CultureInfo.InvariantCulture);
                                            changed = true;
                                            if (!tgasToSize.Contains(tga))
                                            {
                                                tgasToSize.Add(tga);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            foreach (string tga in tgasToSize)
            {
                string tgaIn = Path.Combine(Path.GetDirectoryName(pathInput), tga);
                string tgaOut = Path.Combine(Path.GetDirectoryName(pathOutput), tga);
                if (!File.Exists(tgaOut))
                {
                    ResizeTGA(tgaIn, tgaOut, scale);
                }
            }


            // only write a file if it actually changed things that needed changing
            if (changed)
            {
                // write it out how the default UI is formatted - it doesn't matter to the game but nice to be consistent
                XmlWriterSettings writerSettings = new XmlWriterSettings()
                {
                    CheckCharacters = true,
                    Indent = true,
                    IndentChars = "\t",
                    NewLineChars = "\r\n",
                    Encoding = new UTF8Encoding(false)
                };
                using (XmlWriter writer = XmlWriter.Create(pathOutput, writerSettings))
                {
                    doc.WriteTo(writer);
                    writer.Close();
                }
            }

            return changed;
        }

        private void ResizeTGA(string tgaInput, string tgaOutput, int scale)
        {
            using (MagickImage image = new MagickImage(tgaInput, MagickFormat.Tga))
            {
                //MagickGeometry size = new MagickGeometry(image.Width * scale, image.Height * scale);
                //size.IgnoreAspectRatio = false;

                //image.Resize(size); // this one is blurry
                //image.Resample(size.Width, size.Height);
                //image.Sample(size);
                image.Scale(new Percentage(scale * 100.0)); // this is the best one
                                                            //image.Scale(size); // best
                                                            //image.AdaptiveResize(size);
                                                            //image.Magnify(); // ends up messed up and light colored
                                                            //image.LiquidRescale(size);

                // Save the result
                image.Write(tgaOutput);
            }
        }

        public static string GetXPath_SequentialIteration(XmlElement element)
        {
            string path = "/" + element.Name;

            XmlElement parentElement = element.ParentNode as XmlElement;
            if (parentElement != null)
            {
                // Gets the position within the parent element.
                // However, this position is irrelevant if the element is unique under its parent:
                XmlNodeList siblings = parentElement.SelectNodes(element.Name);
                if (siblings != null && siblings.Count > 1) // There's more than 1 element with the same name
                {
                    int position = 1;
                    foreach (XmlElement sibling in siblings)
                    {
                        if (sibling == element)
                            break;

                        position++;
                    }

                    path = path + "[" + position + "]";
                }

                // Climbing up to the parent elements:
                path = GetXPath_SequentialIteration(parentElement) + path;
            }

            return path;
        }
    }
}
