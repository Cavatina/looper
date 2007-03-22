#ifndef _LOOPER_CHANNEL_H_
#define _LOOPER_CHANNEL_H_

class bank;

class channel
{
public:
	channel(bank *bank_) : bankref(bank_) {}
	virtual ~channel() {}

	void set_name(const std::string &);
	void set_index(unsigned short);

	// Take into account that sample channels might be less
	// than what we want...
	size_t get_audio(void *buf, size_t);

	virtual bool is_input() const { return false; }
	virtual bool is_output() const { return false; }

private:
	bank *bankref;
};

class input_channel : public channel
{
public:
	// TODO: Howto merge several channels... :S
	// TODO: Write along as we go should be the "right thing to do"
	// to avoid excess "XRUN"s...
	void receive_audio(void *buf, size_t);
	virtual bool is_input() const { return true; }

};

class output_channel : public channel
{
public:
	virtual bool is_output() const { return true; }
};

#endif
