#pragma once
#pragma warning(disable:4267)
#include <vector>
#include <string>


enum class MESSAGE_TYPE {
	M_NONE,
	M_TERMINATE,
	M_ERROR,
	M_WARNING,
	M_INFO,
	M_DEBUG,
};

struct Message {
	MESSAGE_TYPE type;
	std::string m;
};

class MessageMaster {
	friend class Poster;
	friend class Listener;
protected:
	static MessageMaster* summon() {
		if (!instance) {
			instance = new MessageMaster();
		}
		return instance;
	}

	std::vector<Message> queue;

private:
	inline static MessageMaster* instance = nullptr;
};

class Poster {
protected:
	Poster() {
		master = master->summon();
	}

	void postMessage(MESSAGE_TYPE type, std::string message) const {
		master->queue.push_back({ type, message });
	}

private:
	MessageMaster* master;
};


class Listener {
protected:
	Listener() {
		master = master->summon();
	}

protected:
	bool peekMessage(Message* m, int i) const {
		if (master->queue.size() > i && i >= 0) {
			*m = master->queue[i];
			return true;
		}
		return false;
	}
	bool popMessage(Message* m = nullptr) const {
		bool empty = true;
		if (m != nullptr && !master->queue.empty()) {
			*m = master->queue.back();
			master->queue.pop_back();
			empty = false;
		}
		return !empty;
	}
	int getQueueSize() const {
		return master->queue.size();
	}

private:
	MessageMaster* master;
};
