/*
  ==============================================================================

    Utilities.h
    Created: 21 Sep 2022 12:48:09pm
    Author:  Nathan Erthal

  ==============================================================================
*/

#pragma once
#include<iostream>
#include<algorithm>

using namespace std;

class Utilities {
public:
    
    int getNoteFromMS(int ms){
        for (int note = 0; note<maxDelay_note; note++){
            if (ms-notesInMS[note] < abs(ms-notesInMS[note+1]))
                return note;
        }
        return maxDelay_note;
    }
    
    int getMSFromNote(int note){
        return notesInMS[note];
    }
    
    void mapNotesToMS(double bpm, int maxDelay_note, int maxDelay_ms){
        this->bpm = bpm;
        
        thirtySecondNoteInMS = (60/bpm/8)*1000;  // 60/bpm = duration of quarter note in s. Divide by 8 for 32nd note value, then mult by 1000 for ms
        
        this->maxDelay_note = maxDelay_note;
        notesInMS.resize(maxDelay_note+1);
        
        for (int note=0; note < maxDelay_note+1; note++){
            notesInMS[note] = notesToMS(note);
        }
        
        this->maxDelay_ms = maxDelay_ms;
    }
    
private:
    
    int bpm;
    float thirtySecondNoteInMS;
    int maxDelay_ms;
    int maxDelay_note;
    std::vector<int> notesInMS; //    vector containing ms for each note(same as index), i guess this is technically a map
    
    float notesToMS(int note){
        float answer =  notesToMSHelper(note) * thirtySecondNoteInMS;
        return min(answer, float(maxDelay_ms));
    }
    
    float notesToMSHelper(int note){
        if (note==0) return 0;
        if (fmod(note,3) == 0)
            {
                //Dotted
                if (note==3) return 1.5;
                else return (2*(notesToMSHelper(note-3)));
            }
        else
        {
            if (fmod(note+2,3) == 0)
            {
                //straight
                if (note==1) return 1;
                else return (2*(notesToMSHelper(note-3)));
            }
            else
            {
                // if (fmod(x+1,3) == 0)
                //TRIPELET
                if (note==2)  return (4.0/3);
                else return (2*(notesToMSHelper(note-3)));
            }
        }
    }
    
};




//    enum NOTES{
//        thirtySecond = 1,
//        thirtySecondDotted = 2,
//        sixteenthTripelet = 3,
//        sixteenth = 4,
//        sixteenthDotted = 5,
//        eighthTripelet = 6,
//        eighth = 7,
//        eighthDotted = 8,
//        quarterTripelet = 9,
//        quarter = 10,
//        quarterDotted = 11,
//        halfTripelet = 12,
//        half = 13,
//        halfDotted = 14,
//        wholeTripelet = 15,
//        whole = 16
//    };
