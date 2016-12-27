#include "ofxAudioDecoder.h"

//----------
bool ofxAudioDecoder::load(ofSoundBuffer & buffer, string filename, size_t framesToRead) {

#ifndef __linux
    // ------------------------ loading using libaudiodecoder ------------------------
	AudioDecoder audioDecoder(ofToDataPath(filename));
	if (audioDecoder.open() != AUDIODECODER_OK) {
		ofLogError() << "Failed to load " << filename;
		return false;
	}
	
	auto numSamples = audioDecoder.numSamples();
	auto numChannels = audioDecoder.channels();
	auto sampleRate = audioDecoder.sampleRate();
	
	buffer.setSampleRate(sampleRate);
	buffer.allocate(numSamples / numChannels, numChannels);
	
	//if user asked for 0 samples
	if(framesToRead == 0) {
		//we interpet that as wanting to read whole file
		framesToRead = numSamples / numChannels;
	}
	int samplesToRead = framesToRead * numChannels;
	int readBufferSize = sampleRate / numChannels;
	
	int curSample = 0;
	auto & rawSamples = buffer.getBuffer();
	while(curSample < samplesToRead) {
		int remainingSamples = MIN(readBufferSize, samplesToRead - curSample);
		int samplesRead = audioDecoder.read(remainingSamples, &rawSamples[curSample]);
		curSample += samplesRead;
		if(samplesRead < readBufferSize) {
			break;
		}
	}

#else
    // ------------------------ linux loading using libsndfile ------------------------
    const char * filePath_c = filename.c_str();  
 
    SndfileHandle fileHandle = SndfileHandle(filePath_c); 
    
    if( strcmp(fileHandle.strError(), "No Error.") !=0 ){
		ofLogError() << "Failed to load " << filename;
		return false;    
    }
	
	auto numSamples = fileHandle.frames();
	auto numChannels = fileHandle.channels();
	auto sampleRate = fileHandle.samplerate();
	
	buffer.setSampleRate(sampleRate);
	buffer.allocate(numSamples / numChannels, numChannels);
	
	//if user asked for 0 samples
	if(framesToRead == 0) {
		//we interpet that as wanting to read whole file
		framesToRead = numSamples / numChannels;
	}
	int samplesToRead = framesToRead * numChannels;
	
    // get the samples to a buffer 
    float* samples;
    try
    {
        samples = new float [numChannels * numSamples];
    }
    catch (std::bad_alloc& ba)
    {
        std::string error = "error loading file: bad_alloc caught: ";
        error += ba.what();
        error +="\n";
        ofLogError() << error <<"\n";
        return false; //abort
    }
    fileHandle.read (samples, numChannels * numSamples);    

	int curSample = 0;  
	auto & rawSamples = buffer.getBuffer();
	while(curSample < samplesToRead) {
        rawSamples[curSample] = samples[curSample];
        curSample++;
	}
    
    delete [] samples;
    
#endif
	
	ofLogVerbose() << "Read " << curSample << " of " << numSamples;
	return true;

}

//----------
bool ofxAudioDecoder::load(string filename, size_t framesToRead) {
	return ofxAudioDecoder::load(this->buffer, filename, framesToRead);
}

//----------
int ofxAudioDecoder::getNumChannels() const {
	return this->buffer.getNumChannels();
}

//----------
int ofxAudioDecoder::getSampleRate() const {
	return this->buffer.getSampleRate();
}

//----------
int ofxAudioDecoder::getNumSamples() const {
	return this->buffer.size();
}

//----------
int ofxAudioDecoder::getNumFrames() const {
	return this->buffer.getNumFrames();
}

//----------
const ofSoundBuffer & ofxAudioDecoder::getBuffer() const {
	return this->buffer;
}

//----------
const vector<float> & ofxAudioDecoder::getRawSamples() const {
	return this->buffer.getBuffer();
}
