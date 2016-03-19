#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <map>
#include <queue>

using namespace std;

int R, C, D_num, T, maxPayLoad, P_num, W_num, O_num;

class ProductType
{
public:
	int ID;	int weight;
	ProductType()
	{

	};
};

class Product
{
public:
	ProductType type;
	int count;
	Product()
	{
		count = 0;
	};
};

class Point
{
public:
	int R;
	int C;
	Point()
	{

	};

	int distanceTo(Point p)
	{
		return (int)ceil(sqrt((p.R - R)*(p.R - R) + (p.C - C)*(p.C - C)));
	}

};

class WareHouse
{
public:
	int ID;
	Point location;
	vector <Product> products;

	WareHouse()
	{
	};
};

class Drone
{
public:
	int free;
	int ID;
	int weight;
	Point location;
	vector <Product> carry;

	Drone()
	{
	};

	friend bool operator < (const Drone &a, const Drone &b)
	{
		return a.free > b.free;
	}

};

vector <WareHouse> warehouses;

class Order
{
private:
	vector <pair<float, int> > closestWareHouseID;
public:
	float h;
	int ID;
	int delived;
	int itemsCount;
	Point location;
	vector <Product> products;

	Order()
	{

	};

	friend bool operator < (Order o1, Order o2)
	{
		return o1.h > o2.h;
	};

	float heuristic1()
	{
		float val = 0;
		for (int i = 0; i < products.size(); i++)
			val += products[i].count * products[i].type.weight;

		val = (int)ceil((double)val / maxPayLoad);
		return val;
	};

	float howMuchOfMyOrderYouHave(int wID)
	{
		float sum = 0.0;
		int cnt = 0;
		for (int i = 0; i < products.size(); i++)
		{
			int pid = products[i].type.ID;
			if (products[pid].count != 0)
			{
				double ratio = warehouses[wID].products[pid].count / products[pid].count;
				ratio = min(ratio, 1.0);
				cnt++;
				sum += ratio;
			}
		}
		if (cnt != 0)
			return sum / cnt;
		return 0;
	}

	//improvment can be
	float heuristic2()
	{
		for (int i = 0; i < W_num; i++)
		{
			int distance = location.distanceTo(warehouses[i].location);
			int ID = warehouses[i].ID;

			float havingRatio = howMuchOfMyOrderYouHave(ID);
			closestWareHouseID.push_back(make_pair((float)distance / havingRatio, ID));
		}
		sort(closestWareHouseID.begin(), closestWareHouseID.end());
		return closestWareHouseID[0].first;
	}

	int getWarehouse(int pID)
	{
		for (int i = 0; i < closestWareHouseID.size(); i++)
		{
			int wID = closestWareHouseID[i].second;
			if (warehouses[wID].products[pID].count != 0)
				return wID;
		}
		return -1;
	};

	void computeHeuristic()
	{
		h = (heuristic1() + heuristic2()) / 2.0f;
	}

	/*void computeClosestWareHouseID()
	{
	for (int i = 0; i < W_num; i++)
	{
	int distance = location.distanceTo(warehouses[i].location);
	int ID = warehouses[i].ID;

	closestWareHouseID.push_back(make_pair(distance, ID));
	}
	sort(closestWareHouseID.begin(), closestWareHouseID.end());
	};*/

};


priority_queue <Order> orders;

vector <ProductType> productTypes;
priority_queue <Drone> drones;


int main()
{
	//string fileName = "redundancy";
	string fileName = "busy_day";
	//string fileName = "mother_of_all_warehouses";
	//string fileName = "input";
	ifstream cin(fileName + ".in");
	ofstream cout(fileName + ".out");

	cin >> R >> C >> D_num >> T >> maxPayLoad;
	cin >> P_num;
	for (int i = 0; i < P_num; i++)
	{
		ProductType p;
		p.ID = i;
		cin >> p.weight;
		productTypes.push_back(p);
	}

	cin >> W_num;
	for (int i = 0; i < W_num; i++)
	{
		WareHouse w;
		w.ID = i;
		cin >> w.location.R >> w.location.C;
		for (int j = 0; j < P_num; j++)
		{
			Product p;
			p.type = productTypes[j];
			cin >> p.count;
			w.products.push_back(p);
		}
		warehouses.push_back(w);
	}

	cin >> O_num;
	for (int i = 0; i < O_num; i++)
	{
		Order o;
		o.ID = i;
		o.delived = 0;
		o.itemsCount = 0;
		cin >> o.location.R >> o.location.C;
		int num_req;
		cin >> num_req;
		for (int j = 0; j < P_num; j++)
		{
			Product p;
			p.type = productTypes[j];
			o.products.push_back(p);
		}

		for (int j = 0; j < num_req; j++)
		{
			int val;
			cin >> val;
			o.products[val].count++;
			o.itemsCount++;
		}


		o.computeHeuristic();
		//o.computeClosestWareHouseID();
		orders.push(o);

	}

	for (int i = 0; i < D_num; i++)
	{
		Drone d;
		d.ID = i;
		d.free = 0;
		d.weight = maxPayLoad;
		d.location.R = warehouses[0].location.R;
		d.location.C = warehouses[0].location.C;
		drones.push(d);
	}

	vector <string> ans;
	while (orders.size() > 0)
	{
		Order o = orders.top();
		orders.pop();
		Drone d = drones.top();
		drones.pop();

		//load part
		vector <string> buffer;
		for (int i = 0; i < o.products.size(); i++)
		{
			int pID = o.products[i].type.ID;
			if (o.products[pID].count != 0)
			{
				int wID = o.getWarehouse(pID);

				//int count = min(o.products[pID].count, warehouses[wID].products[pID].count);
				int count = (int)floor((double)d.weight / o.products[pID].type.weight);
				count = min(o.products[pID].count, count);
				count = min(warehouses[wID].products[pID].count, count);
				if (count != 0)
				{
					int turns = d.location.distanceTo(warehouses[wID].location);
					d.location = warehouses[wID].location;

					Product pCarry;
					pCarry.type = o.products[i].type;
					pCarry.count = count;
					d.carry.push_back(pCarry);

					d.free += turns + 1;//loading time
					d.weight -= o.products[pID].type.weight * count;

					warehouses[wID].products[pID].count -= count;

					string command = to_string(d.ID) + " " + "L" + " " + to_string(wID) + " " + to_string(pID) + " " + to_string(count);
					buffer.push_back(command);
				}
			}
		}

		int turn = d.location.distanceTo(o.location);
		d.free += turn + d.carry.size();

		//Deliver part:
		for (int i = 0; i < d.carry.size(); i++)
		{
			int pid = d.carry[i].type.ID;
			string command = to_string(d.ID) + " " + "D" + " " + to_string(o.ID) + " " + to_string(pid) + " " + to_string(d.carry[i].count);
			buffer.push_back(command);

			o.products[pid].count -= d.carry[i].count;
			o.delived += d.carry[i].count;
			d.weight += d.carry[i].count * d.carry[i].type.weight;
		}



		d.location = o.location;


		//update
		d.carry.clear();
		drones.push(d);

		o.computeHeuristic();
		if (o.delived != o.itemsCount)
			orders.push(o);

		if (d.free > T)
			break;
		else
		{
			for (int i = 0; i < buffer.size(); i++)
				ans.push_back(buffer[i]);
		}
	}

	cout << ans.size() << endl;
	for (int i = 0; i < ans.size(); i++)
		cout << ans[i] << endl;

	return 0;
}
