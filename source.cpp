#include <iostream>
#include <set>
#include <unordered_map>
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <queue>
#include <utility>
#include <forward_list>
using namespace std;
using timePoint = chrono::steady_clock::time_point;

enum commandType {
	eAdd, eRemove, ePrint
};

class Command {
protected:
	commandType type;
	timePoint timestamp;
	vector<int>params;
	string user;
public:
	Command(commandType type,string user, vector<int> params, timePoint timestamp) :type(type), user(user), params(params), timestamp(timestamp) {};
	timePoint getTimestamp() {
		return timestamp;
	}
	vector<int> getParams() {
		return params;
	}
	string getUser() {
		return user;
	}
	commandType getType() {
		return type;
	}
	void add(shared_ptr<multiset<int>> data);
	void remove(shared_ptr<multiset<int>> data);
	void print(shared_ptr<multiset<int>> data);
	void execute(shared_ptr<multiset<int>> data);
};

void Command::execute(shared_ptr<multiset<int>> data)
{
	switch (type)
	{
	case eAdd:
		add(data);
		break;
	case eRemove:
		remove(data);
		break;
	case ePrint:
		print(data);
		break;
	default:
		break;
	}
}

void Command::add(shared_ptr<multiset<int>> data){
	cout << "dodan " << this->params[0] << " dla " << user << endl;
	data->insert(params[0]);
}

void Command::remove(shared_ptr<multiset<int>> data) {
	switch (params.size())
	{
	case 0:
		cout << "usnieto " << user << endl;
		data->clear();
		break;
	case 1:
		cout << "usnieto " << params[0] << " dla " << user << endl;
		data->erase(params[0]);
		break;
	}
}

void Command::print(shared_ptr<multiset<int>> data) {
	switch (params.size())
	{
	case 0:
		cout << user << ": ";
		for (const int& value : *data) {
			std::cout << value << " ";
		}
		cout << endl;
		break;
	case 2:
		cout << user << ": ";
		auto itStart = data->lower_bound(params[0]);
		auto itEnd = data->upper_bound(params[1]);

		for (auto it = itStart; it != itEnd; ++it) {
			std::cout << *it << " ";
		}
		cout << endl;
		break;
	}
}

class Serwer {
private:
	struct CompareCommands {
		bool operator()(const std::shared_ptr<Command>& lhs, const std::shared_ptr<Command>& rhs) const {
			return lhs->getTimestamp() < rhs->getTimestamp();
		}
	};
	unordered_map<string, pair<shared_ptr<mutex>,shared_ptr<multiset<int>>>> usersData;
	priority_queue<shared_ptr<Command>, vector<shared_ptr<Command>>, CompareCommands > commadQueue;
	unordered_map<string, forward_list<shared_ptr<Command>>> pastCommand;
	vector<thread> threads;
	mutex dataMutex;
	mutex queueMutex;
	mutex pastCommandsMutex;
	timePoint start;
	void processing();
	commandType hashCommand(string const& command);

public:
	Serwer(int threadCount): start(chrono::steady_clock::now()){
		for (int i = 0; i < threadCount; i++) {
			threads.emplace_back(&Serwer::processing, this);
		}
	}
	pair<shared_ptr<mutex>, shared_ptr<multiset<int>>> getUserData(string user);
	void entry(string task);
	void print(string user);
	void insertCommand(forward_list<shared_ptr<Command>>& list, shared_ptr<Command> command);
	~Serwer() {
		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}
	}
};

class Clients {
	vector<thread> threads;
	queue<string> tasks;
	mutex queueMutex;
	Serwer& serwer;
	void simulate();
public:
	Clients(int threadCounts,Serwer& serwer, queue<string>& tasks): serwer(serwer), tasks(tasks) {
		for (int i = 0; i < threadCounts; i++) {
			threads.emplace_back(&Clients::simulate, this);
		}
	}

	~Clients() {
		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}
	}
};

int main() {
	queue<string> tasks;
	tasks.push("/user/add?10");
	tasks.push("/resu/add?10");
	tasks.push("/user/add?10");
	tasks.push("/user/add?12");
	tasks.push("/user/add?11");
	tasks.push("/user/print");
	tasks.push("/resu/print");
	tasks.push("/user/print?0?11");
	tasks.push("/user/remove?10");
	tasks.push("/user/print");
	tasks.push("/user/remove");
	tasks.push("/user/print");
	tasks.push("/user/add?151");

	tasks.push("/user/add?101");
	tasks.push("/user/add?121");
	tasks.push("/user/add?111");
	tasks.push("/user/remove?101");
	tasks.push("/user/remove?111");
	tasks.push("/user/remove?121");
	Serwer serwer(5);
	Clients clients(10, serwer, tasks);

	this_thread::sleep_for(1500ms);
	cout << "final check:" << endl;
	serwer.print("user");
	serwer.print("resu");
}

pair<shared_ptr<mutex>, shared_ptr<multiset<int>>> Serwer::getUserData(string user)
{
	unique_lock<mutex> usersDataLock(dataMutex);
	auto it = usersData.find(user);
	if (it != usersData.end()) {
		auto pair = it->second;
		usersDataLock.unlock();
		return pair;
	}
	else {
		pair<shared_ptr<mutex>, shared_ptr<multiset<int>>> pair{ make_shared<mutex>(), make_shared<multiset<int>>()};
		usersData.insert({ user,pair });
		usersDataLock.unlock();
		return pair;
	}
}

void Serwer::entry(string task){
	size_t pos = task.find('/', 1);
	string user = task.substr(1, pos - 1);
	size_t secPos = task.find('?', pos);
	string command = (secPos != string::npos) ? task.substr(pos + 1, secPos - pos - 1) : task.substr(pos + 1, task.length() - pos - 1);
	vector<int> params;
	if (secPos != string::npos) {
		pos = task.find('?', secPos+1);
		if (pos != string::npos) {
			params.push_back(stoi(task.substr(secPos + 1, pos - secPos - 1)));
			params.push_back(stoi(task.substr(pos + 1, task.length() - secPos - 1)));
		}
		else {
			params.push_back(stoi(task.substr(secPos + 1, task.length() - secPos - 1)));
		}
	}
	chrono::steady_clock::time_point timestamp = std::chrono::high_resolution_clock::now();
	lock_guard<mutex> queueLock(queueMutex);
	switch (hashCommand(command))
	{
	case eAdd:
		commadQueue.push(make_shared<Command>(eAdd,user,params,timestamp));
		break;
	case eRemove:
		commadQueue.push(make_shared<Command>(eRemove,user, params, timestamp));
		break;
	case ePrint:
		commadQueue.push(make_shared<Command>(ePrint,user, params, timestamp));
		break;
	}
}

void Serwer::print(string user){
	cout << user << ": ";
	multiset<int> data = *getUserData(user).second;
	for (const int& value : data) {
		cout << value << " ";
	}cout << endl;
}

void Serwer::insertCommand(forward_list<shared_ptr<Command>>& list, shared_ptr<Command> command){
	if (list.empty() || command->getTimestamp() < list.front()->getTimestamp()) {
		list.push_front(command);
		return;
	}

	auto prev = list.before_begin();
	for (auto curr = list.begin(); curr != list.end(); ++curr) {
		if (command->getTimestamp() < curr->get()->getTimestamp()) {
			list.insert_after(prev, command);
			return;
		}
		prev = curr;
	}

	list.insert_after(prev, command);
	return;
}

void Serwer::processing(){
	while (true) {
		auto now = chrono::steady_clock::now();
		unique_lock<mutex> queueLock(queueMutex);
		if (!commadQueue.empty()) {
			shared_ptr<Command> topCommand = commadQueue.top();
			commadQueue.pop();
			queueLock.unlock();

			cout << chrono::duration_cast<std::chrono::microseconds>(topCommand->getTimestamp() - start).count() << " - ";

			lock_guard<mutex> pastCommandsLock(pastCommandsMutex);

			if (chrono::duration_cast<std::chrono::seconds>(now - topCommand->getTimestamp()).count() >= 3) {
				continue;
			}

			if (topCommand->getType() == ePrint) {
				auto pair = getUserData(topCommand->getUser());
				lock_guard<mutex> userLock(*pair.first);
				topCommand->execute(pair.second);
				continue;
			}

			bool isAdd = topCommand->getType() == eAdd;

			string user = topCommand->getUser();
			auto it = pastCommand.find(user);
			timePoint timestamp = topCommand->getTimestamp();

			if (!isAdd) {
				auto pair = getUserData(topCommand->getUser());
				lock_guard<mutex> userLock(*pair.first);
				topCommand->execute(pair.second);
			}

			bool execute = true;

			if (it != pastCommand.end()) {
				for (const auto& commandPtr : pastCommand[user]) {
					if (commandPtr->getTimestamp() >= timestamp) {
						if (isAdd) {
							if (commandPtr->getType() == eRemove) {
								if (commandPtr->getParams().size() == 0 || commandPtr->getParams()[0] == topCommand->getParams()[0]) {
									execute = false;
									break;
								}
							}
						}
						else {
							if (topCommand->getParams().size() == 0) {
								auto pair = getUserData(topCommand->getUser());
								lock_guard<mutex> userLock(*pair.first);
								commandPtr->execute(pair.second);
							}
							else {
								if (topCommand->getType() == eAdd && commandPtr->getParams()[0] == topCommand->getParams()[0]) {
									auto pair = getUserData(topCommand->getUser());
									lock_guard<mutex> userLock(*pair.first);
									commandPtr->execute(pair.second);
								}
							}
						}
					}
				}
			}
			if (isAdd && execute) {
				auto pair = getUserData(topCommand->getUser());
				lock_guard<mutex> userLock(*pair.first);
				topCommand->execute(pair.second);
			}
			if (it != pastCommand.end()) {
				this->insertCommand(pastCommand[user], topCommand);
			}
			else {
				pastCommand[user] = { topCommand };
			}
		}
		else {
			queueLock.unlock();
		}
		
	}
}

commandType Serwer::hashCommand(string const& command)
{
	if (command == "add") return eAdd;
	if (command == "remove") return eRemove;
	if (command == "print") return ePrint;
	throw exception("wrond command");
}

void Clients::simulate(){
	while (true) {
		unique_lock<mutex> queueLock(queueMutex);
		if (!tasks.empty()) {
			string task = tasks.front();
			tasks.pop();  
			queueLock.unlock();
			serwer.entry(task);
		}
		else {
			queueLock.unlock();
			break;
		}
	}
}