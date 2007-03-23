#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <string>
#include <list>
#include <iostream>

#include "preset.h"
#include "metronome.h"
#include "string_split.h"

struct connector_data
{
	int channel;
	std::string name;
	std::string connect;

	bool operator<(const connector_data &b) { return channel < b.channel; }
};

struct tempo_data
{
	bbt when;
	int bpm;
	int beatsperbar;
	int notetype;
};

struct metronome_data
{
	std::list<tempo_data> tempo;
};

struct midi_data
{
	std::list<connector_data> input;
};

struct audio_data
{
	std::list<connector_data> input;
	std::list<connector_data> output;
};

struct config_data
{
	std::string client_name;
	midi_data midi;
	audio_data audio;
	metronome_data metro;
};

static void parse(connector_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"channel"))){
		data.channel = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"name"))){
		data.name = (const char*)s;
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"connect"))){
		data.connect = (const char*)s;
		xmlFree(s);
	}
}

static void parse(midi_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"input")){
			data.input.push_back(connector_data());
			parse(data.input.back(), doc, cur);
		}
		cur = cur->next;
	}
}

static void parse(audio_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"input")){
			data.input.push_back(connector_data());
			parse(data.input.back(), doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"output")){
			data.output.push_back(connector_data());
			parse(data.output.back(), doc, cur);
		}
		cur = cur->next;
	}
}

static void parse(tempo_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"start"))){
		std::deque<int> l = split((const char *)s);
		if(l.size() >= 1) data.when.bar = l[0];
		if(l.size() >= 2) data.when.beat = l[1];
		if(l.size() >= 3) data.when.tick = l[2];
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"bpm"))){
		data.bpm = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"signature"))){
		std::deque<int> l = split((const char *)s);
		if(l.size() >= 1) data.beatsperbar = l[0];
		if(l.size() >= 2) data.notetype = l[1];
		xmlFree(s);
	}
}

static void parse(metronome_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"tempo")){
			data.tempo.push_back(tempo_data());
			parse(data.tempo.back(), doc, cur);
		}
		cur = cur->next;
	}
}

static void parse(config_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"client-name")){
			s = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			data.client_name = (const char*)s;
			xmlFree(s);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"audio")){
			parse(data.audio, doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"midi")){
			parse(data.midi, doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"metronome")){
			parse(data.metro, doc, cur);
		}
		cur = cur->next;
	}
}

struct source_data
{
	int index;
	std::string name;
	int offset;

	bool operator<(const source_data &b) { return index < b.index; }
};

struct bank_data
{
	int index;
	std::string name;
	std::list<connector_data> input;
	std::list<source_data> sources;
};

static void parse(source_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"index"))){
		data.index = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"name"))){
		data.name = (const char*)s;
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"offset"))){
		data.offset = atoi((const char*)s);
		xmlFree(s);
	}
}

static void parse(bank_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"index"))){
		data.index = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"name"))){
		data.name = (const char*)s;
		xmlFree(s);
	}

	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"input")){
			data.input.push_back(connector_data());
			parse(data.input.back(), doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"source")){
			data.sources.push_back(source_data());
			parse(data.sources.back(), doc, cur);
		}
		cur = cur->next;
	}
}

struct looper_data
{
	config_data config;
	std::list<bank_data> banks;
};


void preset::make_backup()
{

}

void preset::read()
{
	if(is_dirty()) save();

	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(get_source().c_str());
	if(!doc) throw file_error(this);

	cur = xmlDocGetRootElement(doc);
	if(!cur){
		xmlFreeDoc(doc);
		throw file_error(this);
	}

	if(xmlStrcmp(cur->name, (const xmlChar *)"looper")){
		xmlFreeDoc(doc);
		throw format_error(this);
	}

	looper_data data;
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(xmlStrcmp(cur->name, (const xmlChar *)"config") == 0){
			parse(data.config, doc, cur);
		}
		else if(xmlStrcmp(cur->name, (const xmlChar *)"bank") == 0){
			data.banks.push_back(bank_data());
			parse(data.banks.back(), doc, cur);
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);

	metronome *metro = obj->get_metronome();
	metro->clear();

	audio_engine *audio =  obj->get_audio_engine();
	audio->set_name(data.config.client_name);
	audio->initialize();

	std::list<bank_data>::iterator i = data.banks.begin();
	for(; i != data.banks.end(); ++i){
		if(i->index < 1 || i->index > (int)data.banks.size())
			throw format_error(this);
	}

	obj->set_banks(data.banks.size());
	i = data.banks.begin();
	for(; i != data.banks.end(); ++i){
		unsigned short channels = i->input.size();
		bank *b = obj->get_bank(i->index);
		b->set_name(i->name);
		i->sources.sort();
		std::list<source_data>::const_iterator j = i->sources.begin();
		for(; j != i->sources.end(); ++j){
			sample *s;
			if(!(s = b->get_sample(j->index))){
				s = new sample();
				s->set_source(j->name);
				s->set_offset(j->offset);
				b->add_sample(s);
			}
			else {
				s->set_source(j->name);
				s->set_offset(j->offset);
			}
			// TODO: channels = max(channels, s->get_channels());
		}

	// TODO: Handle re-read of configuration!

		b->set_channels(channels);
		i->input.sort();
		std::list<connector_data>::const_iterator k = i->input.begin();
		for(; k != i->input.end(); ++k){
			input_channel *c = new input_channel(b);
			c->set_name(k->name);
			c->set_index(k->channel);
			c->set_connect(k->connect);
			b->add_channel(c);
			audio->register_channel(c);
		}
	}

	std::list<tempo_data>::iterator j = data.config.metro.tempo.begin();
	for(; j != data.config.metro.tempo.end(); ++j){
		metro->add(tempo(j->when, j->bpm, j->beatsperbar,
				 j->notetype));
	}
}

void preset::save()
{
}
