#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <string>
#include <list>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "preset.h"
#include "metronome.h"
#include "util/string_split.h"
#include "util/fs.h"
#include "util/ms_time.h"
#include "util/to_string.h"

static const char *default_xml =
"<?xml version=\"1.0\"?>"
"<looper>"
" <config>"
"  <client-name>looper</client-name>"
" </config>"
" <banks>"
"  <bank index=\"1\" name=\"Bank 1\"/>"
"  <bank index=\"2\" name=\"Bank 2\"/>"
"  <bank index=\"3\" name=\"Bank 3\"/>"
"  <bank index=\"4\" name=\"Bank 4\"/>"
" </banks>"
"</looper>";

preset::~preset()
{
	if(is_dirty()) save();
	close_doc();
}

void preset::init_doc()
{
	if(!doc){
		// TODO: Handle opening a .looper directory.
		// (source = directory with a source.looper file in it)

		if(!get_source().size()) throw no_file(this);
		doc = xmlParseFile(get_source().c_str());
		// On failure, check if file exists.
		// If it does, then throw a fatal exception.
		// Otherwise, initialize a new document.
		if(!doc){
			int fd;
			fd = open(get_source().c_str(), O_RDONLY);
			if(fd != -1){
				close(fd);
				throw file_error(this);
			}
			std::string def(getenv("HOME"));
			def += "/.looper/default.looper";
			doc = xmlParseFile(def.c_str());
			if(!doc){
				doc = xmlParseDoc(BAD_CAST default_xml);
			}
			if(doc){
				mark_dirty();
				has_backup = true;
			}
		}
	}
}

void preset::close_doc()
{
	if(doc){
		xmlFreeDoc(doc);
		doc = 0;
	}
}

struct connector_data
{
	connector_data() : channel(-1) {}

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

struct midi_handler_data
{
	midi_handler_data() : controller(-1), param(-1), note(-1) {}

	int controller;
	int param;
	int note;
	std::string command;
};

struct midi_data
{
	midi_data() : channel(-1) {}

	int channel;
	std::list<connector_data> input;
	std::list<midi_handler_data> handlers;
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

static void parse(midi_handler_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"controller"))){
		data.controller = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"param"))){
		data.param = atoi((const char*)s);
		xmlFree(s);
	}
	if((s = xmlGetProp(cur, (const xmlChar *)"note"))){
		data.note = atoi((const char*)s);
		xmlFree(s);
	}
	s = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	data.command = (const char*)s;
	xmlFree(s);
}

static void parse(midi_data &data, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *s;
	if((s = xmlGetProp(cur, (const xmlChar *)"channel"))){
		data.channel = atoi((const char*)s);
		xmlFree(s);
	}
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"input")){
			data.input.push_back(connector_data());
			parse(data.input.back(), doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"handler")){
			data.handlers.push_back(midi_handler_data());
			parse(data.handlers.back(), doc, cur);
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
		std::deque<int> l = split_int((const char *)s);
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
		std::deque<int> l = split_int((const char *)s);
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
	if(has_backup) return;
	std::string p;
	p = fs::make_absolute_path(fs::get_directory_part(get_source()))
		+ "/backup/" + get_source() +
		ms_time::datetime::now().strftime(".%F_%H-%M-%S.backup");
	fs::mkpath(p);
	rename(get_source().c_str(), p.c_str());
	has_backup = true;
}

void preset::read()
{
	init_doc();
	if(is_dirty()) save();

	xmlNodePtr cur;
	cur = xmlDocGetRootElement(doc);
	if(!cur) throw file_error(this);

	if(xmlStrcmp(cur->name, (const xmlChar *)"looper")){
		throw format_error(this);
	}

	looper_data data;
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"config")){
			parse(data.config, doc, cur);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar *)"banks")){
			xmlNodePtr b = cur->xmlChildrenNode;
			while(b != 0){
				if(!xmlStrcmp(b->name, BAD_CAST "bank")){
					data.banks.push_back(bank_data());
					parse(data.banks.back(), doc, b);
				}
				b = b->next;
			}
		}
		cur = cur->next;
	}

	metronome *metro = obj->get_metronome();
	metro->clear();

	std::list<tempo_data>::iterator j = data.config.metro.tempo.begin();
	for(; j != data.config.metro.tempo.end(); ++j){
		metro->add(tempo(j->when, j->bpm, j->beatsperbar,
				 j->notetype));
	}
	audio_engine *audio =  obj->get_audio_engine();
	audio->set_name(data.config.client_name);
	audio->set_metronome(metro);
	audio->initialize();

	std::list<bank_data>::iterator i = data.banks.begin();
	for(; i != data.banks.end(); ++i){
		if(i->index < 1 || i->index > (int)data.banks.size())
			throw format_error(this);
	}

	obj->set_banks(data.banks.size());
	i = data.banks.begin();
	for(; i != data.banks.end(); ++i){
		bank *b = obj->get_bank(i->index);
		b->set_index(i->index);
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
		}

	// TODO: Handle re-read of configuration!

		i->input.sort();
		std::list<connector_data>::const_iterator k = i->input.begin();
		for(; k != i->input.end(); ++k){
			input_channel *c = new input_channel(b);
			c->set_name(k->name);
			if(k->channel >= 0) c->set_index(k->channel);
			c->set_connect(k->connect);
			b->add_channel(c);
			audio->register_channel(c);
		}
	}

	midi_engine *midi = obj->get_midi_engine();
	midi->set_name(data.config.client_name);
	midi->initialize();

	if(data.config.midi.channel >= 0){
		midi->set_channel(data.config.midi.channel);
	}
	std::list<connector_data>::const_iterator k;
	k = data.config.midi.input.begin();
	for(; k != data.config.midi.input.end(); ++k){
		midi->input_connect(k->connect);
	}

	std::list<midi_handler_data>::const_iterator l;
	l = data.config.midi.handlers.begin();
	for(; l != data.config.midi.handlers.end(); ++l){
		command *c = command_parse(obj, l->command);
		midi_handler *m = new midi_handler(c);
		if(l->controller >= 0) m->set_controller(l->controller);
		if(l->param >= 0) m->set_param(l->param);
		if(l->note >= 0) m->set_note(l->note);
		midi->add(m);
	}
}

xmlNodePtr xml_find_index(xmlNodePtr cur, const xmlChar *name, int index)
{
	cur = cur->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, name)){
			xmlChar *s;
			if((s = xmlGetProp(cur, BAD_CAST "index"))){
				int i = atoi((const char*)s);
				xmlFree(s);
				if(i == index) return cur;
			}
		}
		cur = cur->next;
	}
	return 0;
}

xmlNodePtr xml_find_or_create(xmlNodePtr cur, const xmlChar *name, int index)
{
	xmlNodePtr out = xml_find_index(cur, name, index);
	if(!out){
		out = xmlNewChild(cur, 0, name, 0);
		std::string ix = to_string(index);
		xmlSetProp(out, BAD_CAST "index", BAD_CAST ix.c_str());
	}
	return out;
}

void preset::save()
{
	if(doc) make_backup();
	else read();

	looper_data data;

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(!root) throw file_error(this);

	if(xmlStrcmp(root->name, BAD_CAST "looper")){
		throw format_error(this);
	}

	xmlNodePtr cur = root->xmlChildrenNode;
	while(cur != 0){
		if(!xmlStrcmp(cur->name, BAD_CAST "banks")){
			break;
		}
		cur = cur->next;
	}
	if(!cur) cur = xmlNewChild(root, 0, BAD_CAST "banks", 0);

	for(size_t bi=1; bi<=obj->get_banks(); ++bi){
		bank *b = obj->get_bank(bi);
		xmlNodePtr xb = xml_find_or_create(cur, BAD_CAST "bank", bi);
		xmlSetProp(xb, BAD_CAST "name",
			   BAD_CAST b->get_name().c_str());

		// TODO: Update input channels

		for(size_t si=1; si<=b->get_sample_count(); ++si){
			sample *s = b->get_sample(si);
			xmlNodePtr xs;
			xs = xml_find_or_create(xb, BAD_CAST "source", si);
			std::string o(to_string(s->get_offset()));
			xmlSetProp(xs, BAD_CAST "offset", BAD_CAST o.c_str());
			xmlSetProp(xs, BAD_CAST "name",
				   BAD_CAST s->get_source().c_str());
		}
	}

	xmlSaveFormatFile(get_source().c_str(), doc, 1);

	clear_dirty();
}
