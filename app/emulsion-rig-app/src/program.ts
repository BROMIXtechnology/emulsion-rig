/*

So we have the concept of a programme

•⁠  ⁠Speed of addition in drop rate of ml per minute
•⁠  ⁠stirring speed
•⁠  ⁠control by curve
•⁠  ⁠runs for some duration 

•⁠  ⁠step markers, shows along top
•⁠  ⁠String is independent of addition

•⁠  ⁠message alert with beep
•⁠  ⁠beep countdown

•⁠  ⁠metronome beep independent of other functions

•⁠  ⁠maybe prompt 


•⁠  ⁠start program from directory display
•⁠  ⁠pause, resume


01# instruction
01# numeric argument 1
00# numeric argument 2

208001010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010110blahbalhla5hello150oiwajdoiwa


*/

export type EmulsionEvent = {
    eventId: string;
    offset: number;
} & ({
    type: "alert";
    text: string;
    beep: boolean;
} | {
    type: "heading";
    text: string;
} | {
    type: "metronome";
    seconds: number;
    repeat: number; 
} | {
    type: "ramp_speed";
    value: number;
    seconds: number;
} | {
    type: "prompt";
    text: string;
})

export interface EmulsionProgram {
    events: EmulsionEvent[];
}

export interface SdCardContents {
    files: {[filename: string]: EmulsionProgram}
}