using System;
using System.Linq;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

/*　ドメイン層　*/

// 出力アクションの定義
interface IAction
{
    string format();
}

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

// 魚クラス
class Fish
{
    int x, y;
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

/**
 * Score points by scanning valuable fish faster than your opponent.
 **/
class Player
{
    static void Main(string[] args)
    {
        string[] inputs;
        int creatureCount = int.Parse(Console.ReadLine());
        for (int i = 0; i < creatureCount; i++)
        {
            inputs = Console.ReadLine().Split(' ');
            int creatureId = int.Parse(inputs[0]);
            int color = int.Parse(inputs[1]);
            int type = int.Parse(inputs[2]);
        }

        // game loop
        while (true)
        {
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
            for (int i = 0; i < myDroneCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int droneX = int.Parse(inputs[1]);
                int droneY = int.Parse(inputs[2]);
                int emergency = int.Parse(inputs[3]);
                int battery = int.Parse(inputs[4]);
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
            for (int i = 0; i < droneScanCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int creatureId = int.Parse(inputs[1]);
            }
            int visibleCreatureCount = int.Parse(Console.ReadLine());
            int x = 5000, y = 5000;
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
            for (int i = 0; i < radarBlipCount; i++)
            {
                inputs = Console.ReadLine().Split(' ');
                int droneId = int.Parse(inputs[0]);
                int creatureId = int.Parse(inputs[1]);
                string radar = inputs[2];
            }
            for (int i = 0; i < myDroneCount; i++)


                // Write an action using Console.WriteLine()
                // To debug: Console.Error.WriteLine("Debug messages...");

                Console.WriteLine($"Move {x} {y} 1 sss"); // MOVE <x> <y> <light (1|0)> | WAIT <light (1|0)>
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

