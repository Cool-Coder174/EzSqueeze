#include "CompressorProcessor.h"
#include <iostream>
#include <cmath>
#include <vector>

using namespace EzSqueeze::DSP;

int main() {
    std::cout << "EzSqueeze DSP Test Suite" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Test parameters
    const float sampleRate = 44100.0f;
    const int blockSize = 512;
    const int numBlocks = 10;
    
    // Create compressor processor
    CompressorProcessor compressor(sampleRate, blockSize);
    
    // Set up test parameters
    compressor.setThreshold(-6.0f);   // Higher threshold
    compressor.setRatio(2.0f);        // Lower ratio
    compressor.setAttack(1.0f);
    compressor.setRelease(100.0f);
    compressor.setLookahead(2.0f);    // Less lookahead
    compressor.setStereoLink(0.5f);
    compressor.setHPFFrequency(80.0f);
    compressor.setLPFFrequency(8000.0f);
    compressor.setMix(1.0f);
    compressor.setMakeupGain(0.0f);   // No makeup gain initially
    compressor.setAutoMakeup(false);  // Disable auto makeup initially
    compressor.setProgramDependentRelease(false);
    
    // Generate test signal (sine wave with envelope)
    std::vector<float> leftInput(blockSize);
    std::vector<float> rightInput(blockSize);
    std::vector<float> leftOutput(blockSize);
    std::vector<float> rightOutput(blockSize);
    
    float frequency = 440.0f; // A4
    float amplitude = 0.3f;   // Lower amplitude to stay closer to threshold
    
    for (int block = 0; block < numBlocks; ++block) {
        // Generate test signal
        for (int i = 0; i < blockSize; ++i) {
            float t = (block * blockSize + i) / sampleRate;
            float envelope = 0.5f + 0.5f * std::sin(2.0f * M_PI * 0.1f * t); // Slow envelope
            float sample = amplitude * envelope * std::sin(2.0f * M_PI * frequency * t);
            
            leftInput[i] = sample;
            rightInput[i] = sample * 0.8f; // Slightly different for stereo test
        }
        
        // Process audio
        compressor.processStereo(leftInput.data(), rightInput.data(),
                                leftOutput.data(), rightOutput.data(), blockSize);
        
        // Print metering info
        std::cout << "Block " << block << ": ";
        std::cout << "Input: " << compressor.getInputLevel() << " dBFS, ";
        std::cout << "Output: " << compressor.getOutputLevel() << " dBFS, ";
        std::cout << "GR: " << compressor.getGainReduction() << " dB" << std::endl;
    }
    
    // Test latency reporting
    std::cout << "\nLatency: " << compressor.getLatencySamples() << " samples (";
    std::cout << (compressor.getLatencySamples() / sampleRate * 1000.0f) << " ms)" << std::endl;
    
    // Test parameter getters
    std::cout << "\nParameter Verification:" << std::endl;
    std::cout << "Threshold: " << compressor.getThreshold() << " dBFS" << std::endl;
    std::cout << "Ratio: " << compressor.getRatio() << ":1" << std::endl;
    std::cout << "Attack: " << compressor.getAttack() << " ms" << std::endl;
    std::cout << "Release: " << compressor.getRelease() << " ms" << std::endl;
    std::cout << "Lookahead: " << compressor.getLookahead() << " ms" << std::endl;
    std::cout << "Stereo Link: " << (compressor.getStereoLink() * 100.0f) << "%" << std::endl;
    std::cout << "M/S Mode: " << (compressor.getMSMode() ? "On" : "Off") << std::endl;
    std::cout << "HPF: " << compressor.getHPFFrequency() << " Hz" << std::endl;
    std::cout << "LPF: " << compressor.getLPFFrequency() << " Hz" << std::endl;
    std::cout << "Mix: " << (compressor.getMix() * 100.0f) << "%" << std::endl;
    std::cout << "Makeup Gain: " << compressor.getMakeupGain() << " dB" << std::endl;
    std::cout << "Auto Makeup: " << (compressor.getAutoMakeup() ? "On" : "Off") << std::endl;
    std::cout << "Program Dependent Release: " << (compressor.getProgramDependentRelease() ? "On" : "Off") << std::endl;
    
    std::cout << "\nTest completed successfully!" << std::endl;
    
    return 0;
}