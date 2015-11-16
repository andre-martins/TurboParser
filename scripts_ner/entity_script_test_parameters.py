#Run this script, from the project root folder with $ python scripts_ner/EntityScript.py path_to_TurboEntityRecognizer_executable/TurboEntityRecognizer_executable
import sys
import os
import subprocess
import time

dir = os.path.abspath(  os.path.dirname(__file__) )

NumberOfRuns=1
NumberWarmUps=0

RUNNING_TEST=1

if len(sys.argv) == 2:
    Programs = [sys.argv[1]]
    if os.path.isfile(Programs[0]) == False:    
        print 'Program name must be passed in the argument:'
        print 'Ex: python scripts_ner/EntityScript.py path_to_TurboEntityRecognizer_executable/TurboEntityRecognizer_executable'
        sys.exit()
else:
    print 'Program name must be passed in the argument:'
    if os.name == 'nt': 
        print 'Ex: python scripts_ner\EntityScript.py path_to_TurboEntityRecognizer_executable\TurboEntityRecognizer_executable'
    elif os.name == 'posix':
        print 'Ex: python scripts_ner/EntityScript.py path_to_TurboEntityRecognizer_executable/TurboEntityRecognizer_executable'
    else:
        print 'Ex: python scripts_ner/EntityScript.py path_to_TurboEntityRecognizer_executable/TurboEntityRecognizer_executable'
    sys.exit()


timestr                  = time.strftime("%Y%m%d-%H%M%S")
OutputLogFilenamePrefix  = "*T**__LANGUAGE__*_turbo_entityrecogn_run"
OutputLogFilenameSufix   = timestr
OutputLog_Folder         = [os.path.join(dir, '..','ner','logs')]

TrainFiles_Folder        = [os.path.join(dir, '..','ner','data','*__LANGUAGE__*')] 
DevFiles_Folder          = [os.path.join(dir, '..','ner','data','*__LANGUAGE__*')] 
TestFiles_Folder         = [os.path.join(dir, '..','ner','data','*__LANGUAGE__*')] 
GazetteersFiles_Folder   = [os.path.join(dir, '..','ner','data','*__LANGUAGE__*')] 
ModelFiles_Folder        = [os.path.join(dir, '..','ner','models','*__LANGUAGE__*')]
PredictionFiles_Folder   = [os.path.join(dir, '..','ner','results','*__LANGUAGE__*')]

TrainFiles_template      = '*__LANGUAGE__*_train.conll.ner' 
DevFiles_template        = '*__LANGUAGE__*_dev.conll.ner'
TestFiles_template       = '*__LANGUAGE__*_test.conll.ner'
GazetteersFiles_template = '*__LANGUAGE__*_all_gazetteers.txt'
ModelFiles_template      = '*T**__LANGUAGE__*_entityrecogn.model_mo*__MARKOV_ORDER__*_trc*__REGCONST__*_p*__PREFIX__*s*__SUFFIX__*'
PredictionFiles_template = '*T**__LANGUAGE__*_entityrecogn.model_mo*__MARKOV_ORDER__*_trc*__REGCONST__*_p*__PREFIX__*s*__SUFFIX__*.pred'

TrainFiles               = []     
DevFiles                 = []  
TestFiles                = []
GazetteersFiles          = []
ModelFiles               = []
PredictionFiles          = []


Languages = [# 'basque', 'bulgarian','croatian', 'czech', 'danish', 'finnish', 'greek', 'hungarian', 'italian', 'swedish',
'english']
TrainAlgorithm              = ['svm_mira']  #--train_algorithm=svm_mira
TrainRegularizationConstant = ['10','1','0.1','0.01']  #['1.0', '0.1', '0.01']  #--train_regularization_constant=0.01 
TrainEpochs                 = ['5', '50']  #--train_epochs=20 
SequenceModelType           = ['0','1','2']  #--sequence_model_type=0 
FormCutoff                  = ['0']  #--form_cutoff=0 
PrefixLength                = ['0','3']  #['0','2','3']  #--prefix_length=3
SuffixLength                = ['0','3']  #['0','2','3']  #--suffix_length=3
EntityTaggingScheme         = ['bio']
#--logtostderr              


if RUNNING_TEST == 1:
    ModelFiles_template      = ModelFiles_template.replace('*T*', "TEST_")
    PredictionFiles_template = PredictionFiles_template.replace('*T*', "TEST_")
    OutputLogFilenamePrefix  = OutputLogFilenamePrefix.replace('*T*', "TEST_")
else:
    ModelFiles_template      = ModelFiles_template.replace('*T*', "")
    PredictionFiles_template = PredictionFiles_template.replace('*T*', "")
    OutputLogFilenamePrefix  = OutputLogFilenamePrefix.replace('*T*', "")

if len(Languages) == 1:
    OutputLogFilenamePrefix = OutputLogFilenamePrefix.replace('*__LANGUAGE__*', Languages[0])
else:
    OutputLogFilenamePrefix = OutputLogFilenamePrefix.replace('*__LANGUAGE__*', "")
    
if not os.path.exists(OutputLog_Folder[0]):
    os.makedirs(OutputLog_Folder[0])

for language in Languages:    
    temp_folder  = ModelFiles_Folder[0].replace('*__LANGUAGE__*', language)
    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)
    temp_folder  = PredictionFiles_Folder[0].replace('*__LANGUAGE__*', language)
    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)

csv   = open( os.path.join( OutputLog_Folder[0],OutputLogFilenamePrefix+OutputLogFilenameSufix+".csv")   ,"wb")
log   = open( os.path.join( OutputLog_Folder[0],OutputLogFilenamePrefix+OutputLogFilenameSufix+".log")   ,"wb")
err   = open( os.path.join( OutputLog_Folder[0],OutputLogFilenamePrefix+OutputLogFilenameSufix+".err")   ,"wb")
pylog = open( os.path.join( OutputLog_Folder[0],OutputLogFilenamePrefix+OutputLogFilenameSufix+".pylog") ,"wb")

for i in range(len(TrainFiles_Folder)) :
    TrainFiles.append(os.path.join( TrainFiles_Folder[i], TrainFiles_template))
for i in range(len(DevFiles_Folder)) :
    DevFiles.append(os.path.join( DevFiles_Folder[i], DevFiles_template))
for i in range(len(TestFiles_Folder)) :
    TestFiles.append(os.path.join( TestFiles_Folder[i], TestFiles_template))
for i in range(len(GazetteersFiles_Folder)) :
    GazetteersFiles.append(os.path.join( GazetteersFiles_Folder[i], GazetteersFiles_template))
for i in range(len(ModelFiles_Folder)) :
    ModelFiles.append(os.path.join( ModelFiles_Folder[i], ModelFiles_template))
for i in range(len(PredictionFiles_Folder)) :
    PredictionFiles.append(os.path.join( PredictionFiles_Folder[i], PredictionFiles_template))
 
string_to_write=""
string_to_write=string_to_write+"Program; Language; Markov Order; Train Algorithm; Regularization Constant; Train Epochs; Form cutoff; Prefix Length; Suffix Length"
#string_to_write=string_to_write+"; "+"Training time"  #Commented for Windows execution; turn on in Linux environment
if NumberOfRuns == 1:
    #string_to_write=string_to_write+"; "+"Run(Test) time"  #Commented for Windows execution; turn on in Linux environment
    string_to_write=string_to_write+"; "+"CorrectPredict"
    string_to_write=string_to_write+"; "+"Accuracy"
    string_to_write=string_to_write+"; "+"Speed (token/sec)"
else:
    for i in range(NumberOfRuns):
        #string_to_write=string_to_write+"; "+"Run(Test) time["+str(i)+"]"  #Commented for Windows execution; turn on in Linux environment
        string_to_write=string_to_write+"; "+"CorrectPredict["+str(i)+"]"
        string_to_write=string_to_write+"; "+"Accuracy["+str(i)+"]"
        string_to_write=string_to_write+"; "+"Speed["+str(i)+"] (token/sec)"
     
string_to_write=string_to_write+"\n"
csv.write(string_to_write)
     
for program in Programs:
    for language in Languages:    
        for sequence_model_type in SequenceModelType:
            for train_algorithm in TrainAlgorithm:
                for train_regularization_constant in TrainRegularizationConstant:
                    for train_epochs in TrainEpochs:
                        for form_cutoff in FormCutoff: 
                            for prefix_length in PrefixLength:
                                for suffix_length  in  SuffixLength:
                                    for tagging_scheme in EntityTaggingScheme:
                    
                                        TrainFile       = TrainFiles[0]
                                        TrainFile       = TrainFile.replace('*__LANGUAGE__*', language)

                                        DevFile         = DevFiles[0]
                                        DevFile         = DevFile.replace('*__LANGUAGE__*', language)
                                        
                                        TestFile        = TestFiles[0]
                                        TestFile        = TestFile.replace('*__LANGUAGE__*', language)
                                        
                                        GazetteersFile  = GazetteersFiles[0]
                                        GazetteersFile  = GazetteersFile.replace('*__LANGUAGE__*', language)
                                        
                                        ModelFile       = ModelFiles[0]
                                        ModelFile       = ModelFile.replace('*__LANGUAGE__*', language)
                                        ModelFile       = ModelFile.replace('*__MARKOV_ORDER__*', sequence_model_type)
                                        ModelFile       = ModelFile.replace('*__REGCONST__*', train_regularization_constant)
                                        ModelFile       = ModelFile.replace('*__PREFIX__*', prefix_length)
                                        ModelFile       = ModelFile.replace('*__SUFFIX__*', suffix_length)
                                        
                                        PredictionFile  = PredictionFiles[0]
                                        PredictionFile  = PredictionFile.replace('*__LANGUAGE__*', language)
                                        PredictionFile  = PredictionFile.replace('*__MARKOV_ORDER__*', sequence_model_type)
                                        PredictionFile  = PredictionFile.replace('*__REGCONST__*', train_regularization_constant)
                                        PredictionFile  = PredictionFile.replace('*__PREFIX__*', prefix_length)
                                        PredictionFile  = PredictionFile.replace('*__SUFFIX__*', suffix_length)                                            
                                        
                                        #TRAIN
                                        command = []
                                        #command.append("time -p") #Commented for Windows execution; turn on in Linux environment
                                        command.append(program)
                                        command.append("--train")
                                        command.append("--file_train="+TrainFile)
                                        command.append("--file_model="+ModelFile)
                                        command.append("--entity_file_gazetteer="+GazetteersFile)
                                        command.append("--train_algorithm="+train_algorithm)
                                        command.append("--train_regularization_constant="+train_regularization_constant)
                                        command.append("--sequence_model_type="+sequence_model_type)
                                        command.append("--entity_tagging_scheme="+tagging_scheme)
                                        command.append("--train_epochs="+train_epochs)
                                        command.append("--form_cutoff="+form_cutoff)
                                        command.append("--prefix_length="+prefix_length)
                                        command.append("--suffix_length="+suffix_length)
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
                                        command.append("--file_model="+ModelFile)
                                        command.append("--file_test="+TestFile)
                                        command.append("--file_prediction="+PredictionFile)    
                                        command.append("--entity_tagging_scheme="+tagging_scheme)
                                        command.append("--logtostderr")

                                        print "Executing: "
                                        sys.stdout.flush()
                                    
                                        #warm-up X iterations
                                        for iteration in range(0,NumberWarmUps):                                            
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
                                        for iteration in range(NumberOfRuns):
                                            print       "\n"
                                            pylog.write("\n")
                                            log.write(  "\n")
                                            err.write(  "\n")
                                            if NumberOfRuns > 1:
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
                                        string_to_write=string_to_write+";"+sequence_model_type
                                        string_to_write=string_to_write+";"+train_algorithm
                                        string_to_write=string_to_write+";"+train_regularization_constant
                                        string_to_write=string_to_write+";"+train_epochs
                                        string_to_write=string_to_write+";"+form_cutoff
                                        string_to_write=string_to_write+";"+prefix_length
                                        string_to_write=string_to_write+";"+suffix_length
                                        
                                        #string_to_write=string_to_write+";"+str(train_time) #Commented for Windows execution; turn on in Linux environment

                                        for i in range(NumberOfRuns):
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
