#pragma once
#include "IAudioHash.h"

class AudioHash : public IAudioHash{
protected:
	long RawPosition;
	abort_callback_impl abort_callback;
	BYTE MD5[16];
public:
	AudioHash();
protected:
	virtual void PrepareHash(service_ptr_t<file> s);
	void GenerateHash();
};
class Flac : public AudioHash{
public:
	Flac(service_ptr_t<file> s);
};
class Cue : public AudioHash{
private:
	audio_chunk_impl chunk;
	input_helper helper;

protected:
	virtual void PrepareHash(service_ptr_t<file> s= nullptr);
public:
	Cue(const metadb_handle_ptr &track);
};
class Ogg : public AudioHash{
public:
	Ogg(service_ptr_t<file> s);
private:
	bool Validate(BYTE* buf,int offset);
};
class Mpeg : public AudioHash{
public:
	Mpeg(service_ptr_t<file> s);
private:
	bool Validate(int header);
	int ParseVersion(int header);
	int ParseLayer(int header);
	int ParseBitRate(int header);
	int ParseSampleRate(int header);
	int ParseMode(int header);
	int ParseEmphasis(int header);

};
