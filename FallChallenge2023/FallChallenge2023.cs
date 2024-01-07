using System;
using System.Linq;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection.Emit;
using System.Diagnostics;

/*　ドメイン層　*/
// 出力アクションの定義
interface IAction
{
    string format();
}


// 魚クラス
class FishFactory
{
    List<int> colors;
    List<int> types;
    public FishFactory(int size)
    {
        colors = new List<int>(size);
        types = new List<int>(size);
    }
    public void Add(int id, int color, int type)
    {
        Debug.Assert(id < colors.Count && id < types.Count);
        colors[id] = color;
        types[id] = type;
    }
    public IFish GenerateFish(int id, int x, int y)
    {
        int c = colors[id];
        int t = types[id];

        switch (t)
        {
            case 0:
                return new ShallowFish(c, x, y);
            case 1:
                return new MiddleFish(c, x, y);
            case 2:
                return new DeepFish(c, x, y);
            default:
                
        }
    }
}

interface IFish
{ 
    int type { get; }
    const int velocity = 200;
    // territory内に別のFishがいる場合は一番近いFishと反対方向に進む
    const int territory = 600;
}
// end Fish

// Drone
class Drone
{
    // 1ターン当たりの移動速度
    const int maxVelocity = 600;
    // WAITで沈む速度
    const int sinkVelocity = 300;
    const int scanRadius = 800;
    const int poweredScanRadius = 2000;
    // activateされていないときのバッテリーチャージ
    const int battelyCharge = 1;
    const int battelyCapacity = 30;
    // activateされているときのバッテリー消費
    const int battelyConsumption = 5;

    int battely { get; }
    
    Coordinate c { get; }
}

class Field
{
    int width, height;

    public Field(int w, int h)
    {
        this.width = w;
        height = h;
    }

    public bool isInGrid(Coordinate c)
    {
        return 0 <= c.x && c.x < width && 0 <= c.y && c.y < height;
    }
}

class Turn
{
    List<IFish> fishes;

}

/**
 * Score points by scanning valuable fish faster than your opponent.
 **/
class MainClass
{
    static void Main(string[] args)
    {
        string[] inputs;
        int creatureCount = int.Parse(Console.ReadLine());
        var fishF = new FishFactory(creatureCount);
        for (int i = 0; i < creatureCount; i++)
        {
            inputs = Console.ReadLine().Split(' ');
            int creatureId = int.Parse(inputs[0]);
            int color = int.Parse(inputs[1]);
            int type = int.Parse(inputs[2]);

            // fishF.Add(creatureId, color, type);
        }

        int turn = 0;
        // game loop
        while (true)
        {
            turn++;
            int myScore = int.Parse(Console.ReadLine());
            int foeScore = int.Parse(Console.ReadLine());
            int myScanCount = int.Parse(Console.ReadLine());
            var creatures = new HashSet<int>();
            for (int i = 0; i < myScanCount; i++)
            {
                int creatureId = int.Parse(Console.ReadLine());
                creatures.Add(creatureId);
            }
            int foeScanCount = int.Parse(Console.ReadLine());
            for (int i = 0; i < foeScanCount; i++)
            {
                int creatureId = int.Parse(Console.ReadLine());
            }
            int myDroneCount = int.Parse(Console.ReadLine());
            int myBattery = 0, nowX = 0, nowY = 0, myEmergency = 0;
            var myDrones = new HashSet<int>();
            for (int i = 0; i < myDroneCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int droneX = int.Parse(inputs[1]);
                int droneY = int.Parse(inputs[2]);
                int emergency = int.Parse(inputs[3]);
                int battery = int.Parse(inputs[4]);

                nowX = droneX; nowY = droneY;
                myDrones.Add(droneId);
                myBattery = battery;
                myEmergency = emergency;
            }
            int foeDroneCount = int.Parse(Console.ReadLine());
            for (int i = 0; i < foeDroneCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int droneX = int.Parse(inputs[1]);
                int droneY = int.Parse(inputs[2]);
                int emergency = int.Parse(inputs[3]);
                int battery = int.Parse(inputs[4]);
            }
            int droneScanCount = int.Parse(Console.ReadLine());
            var myScanCnt = 0;
            for (int i = 0; i < droneScanCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int creatureId = int.Parse(inputs[1]);
                if (myDrones.Contains(droneId))
                {
                    creatures.Add(creatureId);
                    myScanCnt++;
                }
            }
            int visibleCreatureCount = int.Parse(Console.ReadLine());
            int x = 0, y = 0;
            for (int i = 0; i < visibleCreatureCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int creatureId = int.Parse(inputs[0]);
                int creatureX = int.Parse(inputs[1]);
                int creatureY = int.Parse(inputs[2]);
                int creatureVx = int.Parse(inputs[3]);
                int creatureVy = int.Parse(inputs[4]);
                if (!creatures.Contains(creatureId))
                {
                    x = creatureX + creatureVx;
                    y = creatureY + creatureVy;
                }

            }
            int radarBlipCount = int.Parse(Console.ReadLine());
            int tl = 0, tr = 0, bl = 0, br = 0, maxCnt = 0;
            for (int i = 0; i < radarBlipCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int creatureId = int.Parse(inputs[1]);
                string radar = inputs[2];

                if (creatures.Contains(creatureId))
                {
                    continue;
                }
                switch (radar)
                {
                    case "TL":
                        {
                            tl++;
                            if (tl > maxCnt) maxCnt = tl;
                            break;
                        }
                    case "TR":
                        {
                            tr++;
                            if (tr > maxCnt) maxCnt = tr;
                            break;
                        }
                    case "BL":
                        {
                            bl++;
                            if (bl > maxCnt) maxCnt = bl;
                            break;
                        }
                    case "BR":
                        {
                            br++;
                            if (br > maxCnt) maxCnt = br;
                            break;
                        }
                    default:
                        break;
                }
            }
            for (int i = 0; i < myDroneCount; i++)
            {
                int light = turn % 5 == 0 ? 1 : 0;
                if (x != 0 && y != 0)
                {
                    Console.WriteLine($"Move {x} {y} 0");
                }
                else if (myScanCnt > 1 || maxCnt == 0 || myEmergency == 1)
                {
                    if (myScanCnt > 0) Console.WriteLine($"MOVE {nowX} 499 0 let's save!");
                    else Console.WriteLine($"WAIT 0");
                }
                else if (br == maxCnt)
                {
                    Console.WriteLine($"Move {10000} {10000} {light} {tl} {tr} {bl} {br} battery:{myBattery}");
                }
                else if (bl == maxCnt)
                {
                    Console.WriteLine($"Move {0} {10000} {light} {tl} {tr} {bl} {br} battery:{myBattery}");
                }
                else if (tr == maxCnt)
                {
                    Console.WriteLine($"Move {10000} {0} {light} {tl} {tr} {bl} {br} battery:{myBattery}");
                }
                else if (tl == maxCnt) Console.WriteLine($"Move 0 0 {light} {tl} {tr} {bl} {br} battery:{myBattery}");

                // Write an action using Console.WriteLine()
                // To debug: Console.Error.WriteLine("Debug messages...");

                // MOVE <x> <y> <light (1|0)> | WAIT <light (1|0)>

            }
        }
    }
}

/* 共通ライブラリ */
class Coordinate
{
    public int x { get; }
    public int y { get; }
    public Coordinate(int x, int y)
    {
        this.x = x;
        this.y = y;
    }

    public int Dist(Coordinate c)
    {
        return (int)Math.Floor(Math.Sqrt((c.x - x) * (c.x - x) + (c.y - y) * (c.y - y)));
    }
}

/*ドメイン*/
// action start

class MoveAction : IAction
{
    int x, y, light;
    public MoveAction(int x, int y, int light)
    {
        this.x = x;
        this.y = y;
        this.light = light;
    }

    public string format()
    {
        return $"MOVE {x} {y} {light}";

    }
}

class WaitAction : IAction
{
    int light;
    public WaitAction(int light)
    {
        this.light = light;
    }

    public string format()
    {
        return $"WAIT {light}";
    }
}
// action end
// fish start

class ShallowFish : IFish
{
    int IFish.type => 0;
    const int velocity = 200;
    // territory内に別のFishがいる場合は一番近いFishと反対方向に進む
    const int territory = 600;

    int color;

    int x, y;

    public ShallowFish(int color, int x, int y)
    {
        this.color = color;
        this.x = x;
        this.y = y;
    }
}

class MiddleFish : IFish
{
    int IFish.type => 1;
    const int velocity = 200;
    // territory内に別のFishがいる場合は一番近いFishと反対方向に進む
    const int territory = 600;

    int color;

    int x, y;

    public MiddleFish(int color, int x, int y)
    {
        this.color = color;
        this.x = x;
        this.y = y;
    }
}

class DeepFish : IFish
{
    int IFish.type => 2;
    const int velocity = 200;
    // territory内に別のFishがいる場合は一番近いFishと反対方向に進む
    const int territory = 600;

    int color;

    int x, y;

    public DeepFish(int color, int x, int y)
    {
        this.color = color;
        this.x = x;
        this.y = y;
    }
}
// fish end