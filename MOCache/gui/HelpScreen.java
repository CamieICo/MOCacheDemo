//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package gui;

import java.io.*;
//import javafx.util.*;

import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import javax.swing.JTable;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
// import org.jfree.chart.ChartFactory;
// import org.jfree.chart.ChartFrame;
// import org.jfree.chart.JFreeChart;
// import org.jfree.chart.plot.PlotOrientation;
// import org.jfree.data.category.DefaultCategoryDataset;



public class HelpScreen extends JDialog{

    public final static int GUI_COMMANDS = 0;
    public final static int SECONDO_COMMANDS = 1;
    public final static int SUPPORTEDFORMATS = 2;
    public final static int CELLHEATRANKING = 3;
    public final static int CACHEWARMUPCMD = 4;
    public final static int CACHEBLOCKS = 5;
    //public final static int HITMISS = 6;

    private JScrollPane ScrollPane= new JScrollPane();
    private JList ServerCommands;
    private JList GuiCommands;
    private JList supportedFormats;
    private JList cellHeatRanking;
    private JList CacheWarmupCmd;
    private JPanel cacheBlocks;
   // private JPanel hitMiss;
    private JButton OkBtn = new JButton("close");


    public HelpScreen(Frame F, Vector<String> supportedFormats){
      super(F,false);
      init(supportedFormats);
      getContentPane().setLayout(new BorderLayout());
      JPanel P = new JPanel();
      P.add(OkBtn);
      getContentPane().add(P,BorderLayout.SOUTH);
      getContentPane().add(ScrollPane,BorderLayout.CENTER);
      setSize(400,400);
      OkBtn.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            HelpScreen.this.setVisible(false);
         }
      });

    }

    public void setMode(int mode){
        if (mode==SECONDO_COMMANDS){
            ScrollPane.setViewportView(ServerCommands);
            setTitle("SECONDO commands");
        }
        if (mode==GUI_COMMANDS){
            ScrollPane.setViewportView(GuiCommands);
            setTitle("Gui commands");
        }
        if (mode==SUPPORTEDFORMATS){
            ScrollPane.setViewportView(supportedFormats);
            setTitle("Supported import formats");
        }
        if (mode==CELLHEATRANKING){
            ScrollPane.setViewportView(cellHeatRanking);
            setTitle("3D Cell Heat Ranking");
        }
        if(mode == CACHEWARMUPCMD){
            ScrollPane.setViewportView(CacheWarmupCmd);
            setTitle("Generate Cache Warm-up Commands");
        }
        if(mode == CACHEBLOCKS){
            ScrollPane.setViewportView(cacheBlocks);
            setTitle("Change of Cache Blocks");
        }
//         if(mode == HITMISS){
//             ScrollPane.setViewportView(hitMiss);
//             setTitle("Cache Hit/Miss");
//         }
    }


    private void init(Vector<String> supported){
        Vector SC = new Vector(30);
        SC.add("list databases");
        SC.add("list types");
        SC.add("list type constructors");
        SC.add("list objects");
        SC.add("list operators");
        SC.add("list algebras");
        SC.add("list algebra <identifier>");
        SC.add("--------------");
        SC.add("create database <identifier> ");
        SC.add("delete database <identifier> ");
        SC.add("open database <identifier>");
        SC.add("close database");
        SC.add("save database to <filename>");
        SC.add("restore database <identifier> from <filename>");
        SC.add("extend database <identifier> from <filename>");
        SC.add("extend and update database <identifier> from <filename>");
        SC.add("save object <identifier> to <filename>");
        SC.add("add object <identifier> from <filename>");
        SC.add("export object <identifier> to <filename>");
        SC.add("import object <identifier> from <filename>");
        SC.add("---------------");
        SC.add("type <identifier> = <type expression>");
        SC.add("delete type <identifier>");
        SC.add("create <identifier> : <type expression>");
        SC.add("update <identifier> := <value expression>");
        SC.add("delete <identifier>");
        SC.add("let <identifier> = <value expression>");
        SC.add("query <value expression>");
        SC.add("----------------");
        SC.add("begin transaction");
        SC.add("commit transaction");
        SC.add("abort transaction");
        ServerCommands= new JList(SC);

        Vector GC = new Vector(20);
        GC.add("gui exit");
        GC.add("gui clearAll");
        GC.add("gui addViewer <ViewerName>");
        GC.add("gui selectViewer <ViewerName> ");
        GC.add("gui clearHistory");
        GC.add("gui loadHistory [-r]");
        GC.add("    -r : replace the actual history");
        GC.add("gui saveHistory");
        GC.add("gui showObject <ObjectName>");
        GC.add("gui showAll");
        GC.add("gui hideObject <ObjectName>");
        GC.add("gui hideAll");
        GC.add("gui removeObject <ObjectName>");
        GC.add("gui clearObjectList ");
        GC.add("gui saveObject <ObjectName> ");
        GC.add("gui loadObject ");
        GC.add("gui setObjectDirectory <directory>");
        GC.add("gui loadObjectFrom <FileName>");
        GC.add("    the file should be inthe ObjectDirectory");
        GC.add("gui storeObject <ObjectName>");
        GC.add("gui connect ");
        GC.add("gui disconnect ");
        GC.add("gui status");
        GC.add("gui serverSettings ");
        GC.add("gui renameObject <oldName> -> <newName> ");
        GC.add("gui onlyViewer ");
        GC.add("gui executeFile [-i] filename");
        GC.add("    -i = ignore errors");
        GC.add("gui set debugmode = <true,false>");
        GC.add("gui set timemeasures = <true,false>");
        GC.add("gui set formattedtext = <true,false>");
        GC.add("gui set showCommand = <true,false>");
        GC.add("gui set servertrace = <true,false>");
        GC.add("gui set commandstyle = <tty,gui>");
        GC.add("gui set scriptstyle = <tty,gui>");
        GC.add("-----------------------------");
        GC.add("gui enableOptimizer");
        GC.add("gui disableOptimizer");
        GuiCommands = new JList(GC);

        supportedFormats  = new JList(supported);

        Vector CHR = new Vector(20);
        String path = "/home/camie/secondo/bin/AboutHotData/gridsMap.txt";
        ArrayList<AbstractMap.SimpleEntry<Integer, Integer> > pairs = new ArrayList<>();
        int st = 1;
        try (FileReader reader = new FileReader(path);
        BufferedReader br = new BufferedReader(reader)
        ) {
            String line;
            while ((line = br.readLine()) != null) {
                    // 一次读入一行数据
                pairs.add(new AbstractMap.SimpleEntry<>(st, Integer.parseInt(line)));
                st++;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        Collections.sort(pairs, new Comparator<AbstractMap.SimpleEntry<Integer, Integer>>() {
            @Override
            public int compare(AbstractMap.SimpleEntry<Integer, Integer> o1, AbstractMap.SimpleEntry<Integer, Integer> o2) {
                if (o1.getValue() > o2.getValue()) return -1;
                else if (o1.getValue() < o2.getValue()) return 1;
                return 0;
            }
        });
        String path2 = "/home/camie/secondo/bin/AboutHotData/EveryGridRange.txt";
        ArrayList <String> strs = new ArrayList<>();
        try (FileReader reader = new FileReader(path2);
         BufferedReader br = new BufferedReader(reader)
         ) {
             String line;
             while ((line = br.readLine()) != null) {
                strs.add(line);
             }
         } catch (IOException e) {
            e.printStackTrace();
         }
         String ymdHMS;
         String []arr;
         Vector GCWUM = new Vector(20);
         int preheatNo = 0;
        for(int i = 0; i < pairs.size(); i++){
            ymdHMS = strs.get(pairs.get(i).getKey().intValue() - 1);
            if(preheatNo < 5){
                arr= ymdHMS.split(" ");
                GCWUM.add("let preheatCMD" + preheatNo + " = NJTaxisMBR_rtree2 NJTaxisMBR2 windowintersects [bbox([const upoint value (("
                + "\"" + arr[0] + "\" \"" + arr[3] + "\"" + " TRUE TRUE) (" + arr[1] + " " + arr[2] + " " + arr[4] +" " + arr[5] + "))])] consume;");
                preheatNo++;
            }

            CHR.add(ymdHMS + "     " + pairs.get(i).getValue());
        }
        cellHeatRanking = new JList(CHR);

        CacheWarmupCmd = new JList(GCWUM);

        //cache blocks
        Object[] titles = {"Num", "FileID", "RecordID", "Offset", "Mode", "DataSize", "SlotNo"};
        Vector titlesV = new Vector();
        Vector<Vector> dataV = new Vector<>();
        for (int i = 0; i < titles.length; i++) {
            titlesV.add(titles[i]);
        }
        String path3 = "/home/camie/secondo/bin/CacheRecord/PCache.txt";
        ArrayList <String> strs3 = new ArrayList<>();
        try (FileReader reader = new FileReader(path3);
         BufferedReader br = new BufferedReader(reader) // 建立一个对象，它把文件内容转成计算机能读懂的语言
         ) {
             String line;
             while ((line = br.readLine()) != null) {
                strs3.add(line);
             }
         } catch (IOException e) {
            e.printStackTrace();
         }
         String []arr3;
         for(int i = 0; i < strs3.size(); i++){
            Vector t = new Vector<>();
            arr3 = strs3.get(i).split(" ");
            for (int j = 0; j < arr3.length; j++) {
                t.add(arr3[j]);
            }
            dataV.add(t);
         }
        DefaultTableModel model=new DefaultTableModel(dataV,titlesV);
        JTable table=new JTable(model);
        //table.setBounds(10,150,300,300);
        //创建按钮,刷新表格
        JButton refreshBt = new JButton("Refresh");
        //refreshBt.setBounds(10, 100, 80, 25);
        refreshBt.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Object[] titles = {"Num", "FileID", "RecordID", "Offset", "Mode", "DataSize", "SlotNo"};
                Vector titlesV = new Vector();
                Vector<Vector> dataV = new Vector<>();
                for (int i = 0; i < titles.length; i++) {
                    titlesV.add(titles[i]);
                }
                String path3 = "/home/camie/secondo/bin/CacheRecord/PCache.txt";
                ArrayList <String> strs3 = new ArrayList<>();
                try (FileReader reader = new FileReader(path3);
                 BufferedReader br = new BufferedReader(reader)
                 ) {
                     String line;
                     while ((line = br.readLine()) != null) {
                        strs3.add(line);
                     }
                 } catch (IOException e2) {
                    e2.printStackTrace();
                 }
                 String []arr3;
                 for(int i = 0; i < strs3.size(); i++){
                    Vector t = new Vector<>();
                    arr3 = strs3.get(i).split(" ");
                    for (int j = 0; j < arr3.length; j++) {
                        t.add(arr3[j]);
                    }
                    dataV.add(t);
                 }
                //DefaultTableModel model=new DefaultTableModel(dataV,titlesV);
                DefaultTableModel model = (DefaultTableModel) table.getModel();
                model.setDataVector(dataV,titlesV);
                //table=new JTable(model);
                table.updateUI();
                //table.repaint();
            }
        });
         JPanel panel = new JPanel();
//         JLabel label1 = new JLabel("UsedSize");
//         label1.setBounds(0, 20, 80, 25);
//         panel.add(label1);
//         JLabel label2 = new JLabel("TotalSize");
//         label2.setBounds(10, 50, 80, 25);
//         panel.add(label2);
//         JLabel label3 = new JLabel("Utilization");
//         label3.setBounds(10, 80, 80, 25);
//         panel.add(label3);
//          JPanel p1= new JPanel();
//          JPanel p2= new JPanel();
//         p1.setBorder(BorderFactory.createTitledBorder("Cache Space Usage"));//添加标题边框
//         p2.setBorder(BorderFactory.createTitledBorder("Cache Blocks"));//添加标题边框
//         p1.setBounds(10, 20, 400, 100);
//         p2.setBounds(100, 20, 400, 200);
//         p1.add(label1);
//         p1.add(label2);
//         p1.add(label3);
//         p2.add(refreshBt);
//         p2.add(new JScrollPane(table));
//         panel.add(p1);
//         panel.add(p2);
        panel.add(refreshBt);
        panel.add(new JScrollPane(table));
        cacheBlocks = panel;

//         DefaultCategoryDataset dataset = new DefaultCategoryDataset();
//         dataset.addValue(15, "schools", "1970");
//         dataset.addValue(30, "schools", "1980");
//         dataset.addValue(60, "schools", "1990");
//         dataset.addValue(120, "schools", "2000");
//         dataset.addValue(240, "schools", "2010");
//         dataset.addValue(300, "schools", "2020");
//         dataset.addValue(320, "schools", "2022");
//
//         // 创建JFreeChart对象
//         JFreeChart chart = ChartFactory.createLineChart(
//                 "Example", // 图标题
//                 "Year", // x轴标题
//                 "Schools Count", // y轴标题
//                 dataset, //数据集
//                 PlotOrientation.VERTICAL, //图表方向
//                 false, true, false);
//         // 利用awt进行显示
//         ChartFrame chartFrame = new ChartFrame("Test", chart);
// //         chartFrame.pack();
// //         chartFrame.setVisible(true);
//         JPanel panel2 = new JPanel();
//         panel2.add(chartFrame);
//         hitMiss = panel2;
    }
}



