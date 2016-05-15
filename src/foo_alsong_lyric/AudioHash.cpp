#include "stdafx.h"
#include "AudioHash.h"
#include "md5.h"
AudioHash::AudioHash()
{
	RawPosition = 0;
	Hash.resize(33);
}

void AudioHash::PrepareHash(service_ptr_t<file> s)
{
	s->seek(RawPosition,abort_callback);
	std::vector<BYTE> MD5buf(0x28000);
	int MD5Length = min(MD5buf.size(), (size_t)s->get_size(abort_callback) - RawPosition);
	s->read(MD5buf.data(),MD5Length,abort_callback);
	md5(MD5buf.data(),MD5Length, MD5);
}

void AudioHash::GenerateHash()
{
	CHAR HexArray[] = "0123456789abcdef";
	int i;
	for(i = 0; i < 32; i += 2)
	{
		Hash[i] = HexArray[(MD5[i / 2] & 0xf0) >> 4];
		Hash[i + 1] = HexArray[MD5[i / 2] & 0x0f];
	}
	Hash[i] = 0;
}

Flac::Flac(service_ptr_t<file> s)
{
	PrepareHash(s);
	GenerateHash();
}

void Cue::PrepareHash(service_ptr_t<file> s/*= nullptr*/)
{
	std::vector<double> MD5buf(0x50000);
	while (true)
	{
		// Store the data somewhere.
		audio_sample *sample = chunk.get_data();
		int len = chunk.get_data_length();
		MD5buf.insert(MD5buf.end(), sample, sample + len);
		if(MD5buf.size() > 0x50000)
			break;

		bool decode_done = !helper.run(chunk, abort_callback);
		if (decode_done) break;
	}
	int MD5Length = min(MD5buf.size(), 0x50000)*sizeof(double);
	md5((unsigned char*)MD5buf.data(), MD5Length , MD5);
}

Cue::Cue(const metadb_handle_ptr &track)
{
	file_info_impl info;
	if(!track->get_info_async(info)){
		return;
	}

	const char *realfile = info.info_get("referenced_file");
	if(realfile == nullptr){
		return;
	}
	const char *ttmp = info.info_get("referenced_offset");
	int m, s, ms;
	if(ttmp == NULL)
		m = s = ms = 0;
	else
	{
		std::stringstream stream(ttmp);
		char unused;
		stream >> m >> unused >> s >> unused >> ms;
		const char *pregap = info.info_get("pregap");
		if(pregap)
		{
			std::stringstream stream(pregap);
			int pm, ps, pms;
			stream >> pm >> unused >> ps >> unused >> pms;
			m += pm;
			s += ps;
			ms += pms;
		}
	}
	pfc::string8 str = track->get_path();
	pfc::string realfilename = pfc::io::path::getDirectory(str) + "\\" + realfile;

	// open input
	helper.open(service_ptr_t<file>(), make_playable_location(realfilename.get_ptr(), 0), input_flag_simpledecode, abort_callback);

	helper.get_info(0, info, abort_callback);
	helper.seek(m * 60 + s + ms * 0.01, abort_callback);

	if (!helper.run(chunk, abort_callback)) return;		

	t_uint64 length_samples = audio_math::time_to_samples(info.get_length(), chunk.get_sample_rate());
	//chunk.get_channels();
	PrepareHash();
	GenerateHash();
}

bool Ogg::Validate(BYTE* buf,int offset)
{
	return !memcmp(buf+offset+1,"vorbis",6) && !memcmp(buf+offset+8,"BCV",3);
}

Ogg::Ogg(service_ptr_t<file> s)
{
	s->seek(52,abort_callback);
	BYTE buf[11];

	do{
		s->seek(RawPosition,abort_callback);
		s->read(buf,11,abort_callback);
		if(Validate(buf,0)){
			RawPosition += 11;
			break;
		}
		RawPosition++;
	}while(RawPosition < s->get_size(abort_callback));

	PrepareHash(s);
	GenerateHash();
}

bool Mpeg::Validate(int header)
{
	return ((header >> 21) & 0x7FF) == 0x7FF && ParseVersion(header) != 1 && ParseLayer(header) != 0 && ParseBitRate(header) != 0xF	&& ParseSampleRate(header) != 3	&& ParseMode(header) != 3 && ParseEmphasis(header) != 2;
}

int Mpeg::ParseVersion(int header)
{
	return (header >> 19) & 3;
}

int Mpeg::ParseLayer(int header)
{
	return (header >> 17) & 3;
}

int Mpeg::ParseBitRate(int header)
{
	return (header >> 12) & 0xF;
}

int Mpeg::ParseSampleRate(int header)
{
	return (header >> 10) & 3;
}

int Mpeg::ParseMode(int header)
{
	return (header >> 6) & 3;
}

int Mpeg::ParseEmphasis(int header)
{
	return header & 3;
}

Mpeg::Mpeg(service_ptr_t<file> s)
{
	BYTE buf[10];
	int read;
	do{
		s->seek(RawPosition, abort_callback);
		read = s->read(buf,10,abort_callback);
		if(!memcmp(buf,"ID3",3)){
			RawPosition = (buf[6] << 21 | buf[7] << 14 | buf[8] << 7 | buf[9]) + (size_t)s->get_position(abort_callback);
			read = 0;
		}else{
			break;
		}
		if(read == 0) break;
	}while(read == 10);

	s->seek(RawPosition,abort_callback);

	BYTE hdr[4096];
	// Header
	do{

		read = s->read(hdr,4096,abort_callback);
		for(int i = 3; i < read; i++){
			int header = hdr[i-3];
			if(Validate(header)){
				RawPosition = (size_t)s->get_position(abort_callback) - read + i - 3;
				read = 0;
			}
		}
	}while(read == 4096);

	PrepareHash(s);
	GenerateHash();
}
