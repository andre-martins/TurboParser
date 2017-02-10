#Run this script, from the project root folder with $ python scripts_morph/MorphScript.py path_to_TurboMorphologicalTagger_executable/TurboMorphologicalTagger_executable
import sys
import os
import subprocess
import time
import itertools

dir = os.path.abspath(  os.path.dirname(__file__) )

number_of_runs=1
number_of_warmups=0

running_test=1

if len(sys.argv) == 2:
    Programs = [sys.argv[1]]
    if os.path.isfile(Programs[0]) == False:    
        print 'Program name must be passed in the argument:'
        print 'Ex: python scripts_morph/MorphScript.py path_to_TurboMorphologicalTagger_executable/TurboMorphologicalTagger_executable'
        sys.exit()
else:
    print 'Program name must be passed in the argument:'
    if os.name == 'nt': 
        print 'Ex: python scripts_morph\MorphScript.py path_to_TurboMorphologicalTagger_executable\TurboMorphologicalTagger_executable'
    elif os.name == 'posix':
        print 'Ex: python scripts_morph/MorphScript.py path_to_TurboMorphologicalTagger_executable/TurboMorphologicalTagger_executable'
    else:
        print 'Ex: python scripts_morph/MorphScript.py path_to_TurboMorphologicalTagger_executable/TurboMorphologicalTagger_executable'
    sys.exit()


timestr                     = time.strftime("%Y%m%d-%H%M%S")
output_log_filename_prefix  = "*T**__LANGUAGE__*turbo_morphtagger_run"
output_log_filename_sufix   = timestr
output_log_folder           = [os.path.join(dir, '..','data_local','morph_log')]

train_files_Folder          = [os.path.join(dir, '..','data_local','morph_data')] 
dev_files_Folder            = [os.path.join(dir, '..','data_local','morph_data')] 
test_files_Folder           = [os.path.join(dir, '..','data_local','morph_data')] 
model_files_Folder          = [os.path.join(dir, '..','data_local','morph_models')]
prediction_files_Folder     = [os.path.join(dir, '..','data_local','morph_out')]

train_files_template        = '*__LANGUAGE__*-ud-train.conllu'
dev_files_template          = '*__LANGUAGE__*-ud-dev.conllu'
test_files_template         = '*__LANGUAGE__*-ud-test.conllu'
model_files_template        = '*T**__LANGUAGE__*_morphtagger.model_mo*__MARKOV_ORDER__*_feat*__FEATURES__*_trc*__REGCONST__*_p*__PREFIX__*s*__SUFFIX__*'
prediction_files_template   = '*T**__LANGUAGE__*_morphtagger.model_mo*__MARKOV_ORDER__*_feat*__FEATURES__*_trc*__REGCONST__*_p*__PREFIX__*s*__SUFFIX__*.pred'

train_files                 = []     
dev_files                   = []   
test_files                  = []
model_files                 = []
prediction_files            = []


languages = ['basque',
'bulgarian',
'croatian',
'czech',
'danish',
'english',
'finnish',
'greek',
'hungarian',
'italian',
'swedish']
morph_features_picker           = ['0']  #['0', '1', '2']  #--morph_tagger_large_feature_set=0
train_algorithms                = ['svm_mira']  #--train_algorithm=svm_mira
train_regularization_constants  = ['0.01']  #['1.0', '0.1', '0.01']  #--train_regularization_constant=0.01 
train_epochs_picker             = ['20']  #--train_epochs=20 
sequence_model_types            = ['0']  #--sequence_model_type=0 
form_cutoffs                    = ['0']  #--form_cutoff=0 
prefix_lengths                  = ['0']  #['0','2','3']  #--prefix_length=3 
suffix_lengths                  = ['3']  #['0','2','3']  #--suffix_length=3 
#--logtostderr              


if running_test == 1:
    model_files_template        = model_files_template.replace('*T*', "TEST_")
    prediction_files_template   = prediction_files_template.replace('*T*', "TEST_")
    output_log_filename_prefix  = output_log_filename_prefix.replace('*T*', "TEST_")
else:
    model_files_template        = model_files_template.replace('*T*', "")
    prediction_files_template   = prediction_files_template.replace('*T*', "")
    output_log_filename_prefix  = output_log_filename_prefix.replace('*T*', "")

if len(languages) == 1:
    output_log_filename_prefix  = output_log_filename_prefix.replace('*__LANGUAGE__*', languages[0])
else:
    output_log_filename_prefix  = output_log_filename_prefix.replace('*__LANGUAGE__*', "")
    
if not os.path.exists(output_log_folder[0]):
    os.makedirs(output_log_folder[0])

for language in languages:    
    temp_folder  = model_files_Folder[0].replace('*__LANGUAGE__*', language)
    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)
    temp_folder  = prediction_files_Folder[0].replace('*__LANGUAGE__*', language)
    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)
        
csv   = open( os.path.join( output_log_folder[0],output_log_filename_prefix+output_log_filename_sufix+".csv")   ,"wb")
log   = open( os.path.join( output_log_folder[0],output_log_filename_prefix+output_log_filename_sufix+".log")   ,"wb")
err   = open( os.path.join( output_log_folder[0],output_log_filename_prefix+output_log_filename_sufix+".err")   ,"wb")
pylog = open( os.path.join( output_log_folder[0],output_log_filename_prefix+output_log_filename_sufix+".pylog") ,"wb")

for i in range(len(train_files_Folder)) :
    train_files.append(os.path.join( train_files_Folder[i], train_files_template))
for i in range(len(dev_files_Folder)) :
    dev_files.append(os.path.join( dev_files_Folder[i], dev_files_template))
for i in range(len(test_files_Folder)) :
    test_files.append(os.path.join( test_files_Folder[i], test_files_template))
for i in range(len(model_files_Folder)) :
    model_files.append(os.path.join( model_files_Folder[i], model_files_template))
for i in range(len(prediction_files_Folder)) :
    prediction_files.append(os.path.join( prediction_files_Folder[i], prediction_files_template))
 
string_to_write=""
string_to_write=string_to_write+"Program; Language; Features; Markov Order; Train Algorithm; Regularization Constant; Train Epochs; Form cutoff; Prefix Length; Suffix Length"
#string_to_write=string_to_write+"; "+"Training time"  #Commented for Windows execution; turn on in Linux environment
if number_of_runs == 1:
    #string_to_write=string_to_write+"; "+"Run(Test) time"  #Commented for Windows execution; turn on in Linux environment
    string_to_write=string_to_write+"; "+"CorrectPredict"
    string_to_write=string_to_write+"; "+"Accuracy"
    string_to_write=string_to_write+"; "+"Speed (token/sec)"
else:
    for i in range(number_of_runs):
        #string_to_write=string_to_write+"; "+"Run(Test) time["+str(i)+"]"  #Commented for Windows execution; turn on in Linux environment
        string_to_write=string_to_write+"; "+"CorrectPredict["+str(i)+"]"
        string_to_write=string_to_write+"; "+"Accuracy["+str(i)+"]"
        string_to_write=string_to_write+"; "+"Speed["+str(i)+"] (token/sec)"
     
string_to_write=string_to_write+"\n"
csv.write(string_to_write)

for program, language, features, sequence_model_type, train_algorithm, train_regularization_constant, train_epochs, form_cutoff, prefix_length, suffix_length in itertools.product(Programs, languages, morph_features_picker, sequence_model_types, train_algorithms, train_regularization_constants, train_epochs_picker, form_cutoffs, prefix_lengths, suffix_lengths):
    train_file       = train_files[0]
    train_file       = train_file.replace('*__LANGUAGE__*', language)
    
    dev_file         = dev_files[0]
    dev_file         = dev_file.replace('*__LANGUAGE__*', language)
    
    test_file        = test_files[0]
    test_file        = test_file.replace('*__LANGUAGE__*', language)
    
    model_file       = model_files[0]
    model_file       = model_file.replace('*__LANGUAGE__*', language)
    model_file       = model_file.replace('*__MARKOV_ORDER__*', sequence_model_type)
    model_file       = model_file.replace('*__FEATURES__*', features)
    model_file       = model_file.replace('*__REGCONST__*', train_regularization_constant)
    model_file       = model_file.replace('*__PREFIX__*', prefix_length)
    model_file       = model_file.replace('*__SUFFIX__*', suffix_length)
    
    prediction_file  = prediction_files[0]
    prediction_file  = prediction_file.replace('*__LANGUAGE__*', language)
    prediction_file  = prediction_file.replace('*__MARKOV_ORDER__*', sequence_model_type)
    prediction_file  = prediction_file.replace('*__FEATURES__*', features)
    prediction_file  = prediction_file.replace('*__REGCONST__*', train_regularization_constant)
    prediction_file  = prediction_file.replace('*__PREFIX__*', prefix_length)
    prediction_file  = prediction_file.replace('*__SUFFIX__*', suffix_length)                                            
    
    #TRAIN
    command = []
    #command.append("time -p") #Commented for Windows execution; turn on in Linux environment
    command.append(program)
    command.append("--train")
    command.append("--file_train="+train_file)
    command.append("--file_model="+model_file)
    command.append("--train_algorithm="+train_algorithm)
    command.append("--train_regularization_constant="+train_regularization_constant)
    command.append("--train_epochs="+train_epochs)
    command.append("--sequence_model_type="+sequence_model_type)
    command.append("--form_cutoff="+form_cutoff)
    command.append("--prefix_length="+prefix_length)
    command.append("--suffix_length="+suffix_length)
    command.append("--morph_tagger_large_feature_set="+features)
    command.append("--logtostderr")

    print "Executing: "
    sys.stdout.flush()

    print       ' '.join(command)
    sys.stdout.flush()
    pylog.write(' '.join(command) + "\n")
    log.write(  ' '.join(command) + "\n")
    err.write(  ' '.join(command) + "\n")
    #run program
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout__output,stderr_output) = process.communicate()
    print "Finished executing: " + ' '.join(command)
    sys.stdout.flush()
        
    # print "- - - * * * - - -"
    stdout__output = stdout__output.splitlines()
    for line in stdout__output:
        log.write(line + "\n")
        # print line.rstrip("\n")
            
    # print "- - - * * * - - -"
    stderr_output = stderr_output.splitlines()
    for line in stderr_output:
        err.write(line + "\n")
        # print line.rstrip("\n")                                        
        where=line.find("Training took ")
        if where != -1:
            print line[where:]
            sys.stdout.flush()
            training_time = float(line[where+len("Training took "):len(line)-len(" sec.")])
            print       "Training time = "+str(training_time)+" seconds\n"
            sys.stdout.flush()
            pylog.write("Training time = "+str(training_time)+" seconds\n")
            log.write(  "Training time = "+str(training_time)+" seconds\n")
            err.write(  "Training time = "+str(training_time)+" seconds\n")

        if line[0:4]=="real":
           train_time = float(line[5:])
           print       "time of execution = "+str(train_time)+" seconds\n"
           sys.stdout.flush()
           pylog.write("time of execution = "+str(train_time)+" seconds\n")
           log.write(  "time of execution = "+str(train_time)+" seconds\n")
           err.write(  "time of execution = "+str(train_time)+" seconds\n")
        # print line
    
    #TEST
    command = []
    #command.append("time -p") #Commented for Windows execution; turn on in Linux environment
    command.append(program)        
    command.append("--test")        
    command.append("--evaluate")
    command.append("--file_model="+model_file)
    command.append("--file_test="+dev_file)
    command.append("--file_prediction="+prediction_file)    
    command.append("--logtostderr")

    print "Executing: "
    sys.stdout.flush()

    #warm-up X iterations
    for iteration in range(0,number_of_warmups):                                            
        print       "Warm-up #" + str(iteration+1) +": " + ' '.join(command)
        sys.stdout.flush()
        pylog.write("Warm-up #" + str(iteration+1) +": " + ' '.join(command) + "\n")
        log.write(  "Warm-up #" + str(iteration+1) +": " + ' '.join(command) + "\n")
        err.write(  "Warm-up #" + str(iteration+1) +": " + ' '.join(command) + "\n")
        #run program
        process=subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout__output,stderr_output) = process.communicate()
        print "Finished executing: " + ' '.join(command)
        sys.stdout.flush()

    test_time=[]
    correct_predictions=[]
    accuracy=[]
    tagspeed=[]
    for iteration in range(number_of_runs):
        print       "\n"
        pylog.write("\n")
        log.write(  "\n")
        err.write(  "\n")
        if number_of_runs > 1:
            print       "\n**** ITER "+str(iteration)+" ****\n"
            sys.stdout.flush()
            pylog.write("\n**** ITER "+str(iteration)+" ****\n")
            log.write(  "\n**** ITER "+str(iteration)+" ****\n")
            err.write(  "\n**** ITER "+str(iteration)+" ****\n")
        print       ' '.join(command)
        sys.stdout.flush()
        pylog.write(' '.join(command) + "\n")
        log.write(  ' '.join(command) + "\n")
        err.write(  ' '.join(command) + "\n")
        #run program
        process=subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout__output,stderr_output) = process.communicate()
        print "Finished executing: " + ' '.join(command)
        sys.stdout.flush()
        
        # print "- - - * * * - - -"
        stdout__output = stdout__output.splitlines()                                      
        for line in stdout__output:
            log.write(line + "\n")
            # print line.rstrip("\n")
        # print "- - - * * * - - -"
        stderr_output = stderr_output.splitlines()
        for line in stderr_output:
            err.write(line + "\n")
            # print line.rstrip("\n")
            where=line.find("Correct predictions:")  
            if where != -1:    
                print line[where:]
                sys.stdout.flush()
                correct_predictions.append( line[where+len("Correct predictions: "):] )
                print       "Correct predictions: "+correct_predictions[iteration]+"\n"
                sys.stdout.flush()
                pylog.write("Correct predictions: "+correct_predictions[iteration]+"\n")
                log.write(  "Correct predictions: "+correct_predictions[iteration]+"\n")  
                err.write(  "Correct predictions: "+correct_predictions[iteration]+"\n")  
            
            where=line.find("Tagging accuracy: ")
            if where != -1:
                print line[where:]
                sys.stdout.flush()
                accuracy.append( float(line[where+len("Tagging accuracy: "):]) )
                print       "Tagging accuracy = "+str(accuracy[iteration])+"\n"
                sys.stdout.flush()
                pylog.write("Tagging accuracy = "+str(accuracy[iteration])+"\n")
                log.write(  "Tagging accuracy = "+str(accuracy[iteration])+"\n")
                err.write(  "Tagging accuracy = "+str(accuracy[iteration])+"\n")
           
            where=line.find("Tagging speed: ")  
            if where != -1:    
                print line[where:]
                sys.stdout.flush()
                tagspeed.append( float(line[where+len("Tagging speed: "):len(line) - len(" tokens per second.")]) )
                print       "Tagging speed = "+str(tagspeed[iteration])+"\n"
                sys.stdout.flush()
                pylog.write("Tagging speed = "+str(tagspeed[iteration])+"\n")
                log.write(  "Tagging speed = "+str(tagspeed[iteration])+"\n")
                err.write(  "Tagging speed = "+str(tagspeed[iteration])+"\n")
            
            if line[0:4]=="real":
                test_time.append( float(line[5:]) )
                print       "time of execution = "+str(test_time[iteration])+" seconds\n"
                sys.stdout.flush()
                pylog.write("time of execution = "+str(test_time[iteration])+" seconds\n")
                log.write(  "time of execution = "+str(test_time[iteration])+" seconds\n")
                err.write(  "time of execution = "+str(test_time[iteration])+" seconds\n")
            # print line    
            
    string_to_write=""
    string_to_write=string_to_write+program
    string_to_write=string_to_write+";"+language
    string_to_write=string_to_write+";"+features
    string_to_write=string_to_write+";"+sequence_model_type
    string_to_write=string_to_write+";"+train_algorithm
    string_to_write=string_to_write+";"+train_regularization_constant
    string_to_write=string_to_write+";"+train_epochs
    string_to_write=string_to_write+";"+form_cutoff
    string_to_write=string_to_write+";"+prefix_length
    string_to_write=string_to_write+";"+suffix_length
    
    #string_to_write=string_to_write+";"+str(train_time) #Commented for Windows execution; turn on in Linux environment

    for i in range(number_of_runs):
        #string_to_write=string_to_write+";"+str(test_time[i]) #Commented for Windows execution; turn on in Linux environment
        string_to_write=string_to_write+";"+str(correct_predictions[i])
        string_to_write=string_to_write+";"+str(accuracy[i])
        string_to_write=string_to_write+";"+str(tagspeed[i])
    string_to_write=string_to_write+"\n"
    
    csv.write(string_to_write)

    csv.flush()
    log.flush()
    pylog.flush()
    err.flush()

#CLOSING						
csv.close()
print "Script finished\n"
log.write("Script finished\n")
pylog.write("Script finished\n")
err.write("Script finished\n")
log.close()
pylog.close()
err.close()
