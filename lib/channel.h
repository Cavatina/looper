#ifndef _LOOPER_CHANNEL_H_
#define _LOOPER_CHANNEL_H_

class bank;

class channel
{
public:
	channel(bank *bank_) : bankref(bank_) {}
	virtual ~channel() {}

	void set_name(const std::string &name_) { name = name_; }
	void set_connect(const std::string &connect_) { connect = connect_; }
	// deprecated?
	void set_index(unsigned short index_) { index = index_; }

	// Take into account that sample channels might be less
	// than what we want...
	size_t get_audio(void *buf, size_t);

	virtual bool is_input() const { return false; }
	virtual bool is_output() const { return false; }

private:
	bank *bankref;
	std::string name;
	std::string connect;
	unsigned short index;
};

class input_channel : public channel
{
public:
	input_channel(bank *bank_) : channel(bank_) {}
	virtual ~input_channel() {}
	// TODO: Howto merge several channels... :S
	// TODO: Write along as we go should be the "right thing to do"
	// to avoid excess "XRUN"s...
	void receive_audio(void *buf, size_t);
	virtual bool is_input() const { return true; }

};

class output_channel : public channel
{
public:
	output_channel(bank *bank_) : channel(bank_) {}
	virtual ~output_channel() {}

	virtual bool is_output() const { return true; }
};

#endif
