#! /bin/python

def process_input(input_str):
    # Define the tone length constant
    TONE_LENGTH = 1  # This can be adjusted if needed

    # Parse the input
    notes = input_str.split(', ')
    
    # Initialize an empty list for formatted notes
    formatted_notes = []

    # Process each note in the list
    for note in notes:
        # Check if the note has a duration (i.e., it ends with a number)
        vals = note.split(' ', 1)
        if len(vals) > 1:
            # Separate the note from its duration
            note_value = vals[0]  # Remove the last two characters (duration and space)
            duration = vals[1]
            formatted_notes.append(f'{{NOTE_{note_value}, TONE_LENGTH {duration}}}')
        elif note[0] == '0':
            # Default duration is TONE_LENGTH
            formatted_notes.append(f'{{0, TONE_LENGTH}}')
        else:
            formatted_notes.append(f'{{NOTE_{note}, TONE_LENGTH}}')
    
    # Join the formatted notes into the desired output format
    output = ', '.join(formatted_notes)
    
    return output

filename = 'songs.txt'

# Open the file in read mode
with open(filename, 'r') as file:
    # Iterate over each line in the file
    for line in file:
        # Print the line
        print('{' + process_input(line) + '},\n', end='')

