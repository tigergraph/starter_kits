import os
import zipfile
import re
import gzip
import shutil

DEBUG = 'e'

def pdebug(value,level):
     if DEBUG == level or DEBUG == 'a':
            print(value)


def zip2structure(filepath):
    # name of zip
    zip_name = filepath.split('/')[-1].replace('.zip','',1)

    # make necessary folders
    raw_starter_kits = os.path.join(os.getcwd(), 'raw_starter_kits')
    if not os.path.exists(raw_starter_kits):
        os.mkdir(raw_starter_kits)

    starter_kits = os.path.join(os.getcwd(), 'starter_kits')
    if not os.path.exists(starter_kits):
        os.mkdir(starter_kits)

    unzipped_raw = os.path.join(raw_starter_kits, zip_name+'_raw')
    # unzip into unzipped_raw
    with zipfile.ZipFile(filepath, 'r') as zip:
        zip.extractall(unzipped_raw)

    # make structure
    unzipped_dirpath = os.path.join(starter_kits, zip_name)
    if not os.path.exists(unzipped_dirpath):
        os.mkdir(unzipped_dirpath)
        with open(os.path.join(unzipped_dirpath, 'README.md'), 'w') as handler:
            handler.write("Documentation")
        for subdir in ['data', 'db_scripts', 'notebooks']:
            os.mkdir(os.path.join(unzipped_dirpath, subdir))
        for subdir in ['jobs', 'queries', 'schemas','UDFs']:
            os.mkdir(os.path.join(unzipped_dirpath,'db_scripts',subdir))


    # get graph name
    DBImportExport_location = ''
    graph_name = ''
    for file in os.listdir(os.path.join(os.getcwd(), unzipped_raw)):
        if file.startswith('DBImportExport'):
            DBImportExport_location = os.path.join(unzipped_raw, file)
    with open(DBImportExport_location) as handler:
        first_line = handler.readline()
        # graph_name = re.match(r'CREATE GRAPH [^\s]* ?\(', first_line).group(0)[len('CREATE GRAPH') + 1 : first_line.find('(')]
        graph_name = first_line[len('CREATE GRAPH') + 1 : first_line.find('(')]
    
    # move data
    data_location_src = os.path.join(unzipped_raw, 'GlobalTypes')
    data_location_dest = os.path.join(unzipped_dirpath, 'data')
    with zipfile.ZipFile(os.path.join(data_location_dest,'data.zip'), 'w') as datazip: 
        for file in os.listdir(data_location_src):
            if file.endswith('.csv'):
                # os.rename(os.path.join(data_location_src, file), os.path.join(data_location_dest, file))
                datazip.write(os.path.join(data_location_src, file), file)
        with open(os.path.join(data_location_dest,'data.zip'), 'rb') as infile:
            with gzip.open(os.path.join(data_location_dest,'data.zip.gz'), 'wb') as outfile:
                shutil.copyfileobj(infile, outfile)
    
    # move UDFs ?
    UDFs_location_src = os.path.join(unzipped_raw, 'ExprFunctions.hpp')
    UDFs_location_dest = os.path.join(unzipped_dirpath, 'db_scripts', 'UDFs', 'ExprFunctions.hpp')
    os.rename(UDFs_location_src, UDFs_location_dest)

    # move/modify schema 
    schema_location_src = os.path.join(unzipped_raw, 'global.gsql')
    schema_location_dest = os.path.join(unzipped_dirpath, 'db_scripts', 'schemas', 'schema.gsql')
    os.rename(schema_location_src, schema_location_dest)
    
    handler = open(schema_location_dest)
    schema_lines = [line.rstrip('\n') for line in handler]
    pdebug(schema_lines, 'i')
    for i in range(len(schema_lines)):
        schema_lines[i] += ';'
        line = schema_lines[i].split()
        line[0] = 'ADD'
        schema_lines[i] = ' '.join(line)

    overwrite_schema =f'CREATE GRAPH {graph_name} ()\nCREATE SCHEMA_CHANGE JOB change_schema_of_{graph_name} FOR GRAPH {graph_name} {{\n'
    for line in schema_lines:
        overwrite_schema += ('\t' + line + '\n')
    overwrite_schema += f'}}\nRUN SCHEMA_CHANGE JOB change_schema_of_{graph_name}\nDROP JOB change_schema_of_{graph_name}'
    pdebug(overwrite_schema,'i')
    handler.close()

    handler = open(schema_location_dest, 'w')
    handler.write(overwrite_schema)
    handler.close()

    # move queries and jobs into respective files
    DBImportExport_code = ''
    with open(DBImportExport_location) as handler:
        DBImportExport_code = handler.read().replace('set exit_on_error = "true"', '').replace('set exit_on_error = "false"', '')
    DBImportExport_code = DBImportExport_code.split('\n')
    indicies_queries, queries = [], []
    indicies_jobs, jobs = [], []
    for element in DBImportExport_code:
        if 'CREATE QUERY' in element:
            indicies_queries.append(DBImportExport_code.index(element))
        if 'CREATE LOADING JOB' in element:
            indicies_jobs.append(DBImportExport_code.index(element))

    if len(indicies_queries) > 0:
        for i in range(len(indicies_queries) - 1):
            queries.append("\n".join(DBImportExport_code[indicies_queries[i] : indicies_queries[i+1]]))
        queries.append("\n".join(DBImportExport_code[indicies_queries[-1]:]))
    if len(indicies_jobs) > 0:
        for i in range(len(indicies_jobs) - 1):
            jobs.append("\n".join(DBImportExport_code[indicies_jobs[i] : indicies_jobs[i+1]]))
        if len(indicies_queries) > 0:
            jobs.append("\n".join(DBImportExport_code[indicies_jobs[-1]: indicies_queries[0]]))
        else:
            jobs.append("\n".join(DBImportExport_code[indicies_jobs[-1]:]))


    jobs_location_dest = os.path.join(unzipped_dirpath, 'db_scripts', 'jobs')
    for i, job in enumerate(jobs):
        first_line = DBImportExport_code[indicies_jobs[i]]

        _, sep, sub2 = first_line.partition('load_job_')
        job_name = sub2[:sub2.find('_')]
        job_file_name = sep + job_name + '.gsql'
        job = re.sub('load_job_[^\s]*', job_name + 'Job', job)

        with open(os.path.join(jobs_location_dest, job_file_name), 'w') as handler:
            handler.write(job)

    queries_location_dest = os.path.join(unzipped_dirpath, 'db_scripts', 'queries')
    for i, query in enumerate(queries):
        first_line = DBImportExport_code[indicies_queries[i]]
        query_name = first_line[len('CREATE GRAPH') + 1 : first_line.find('(')] + '.gsql'
        with open(os.path.join(queries_location_dest, query_name), 'w') as handler:
            handler.write(query)


def main():
    zipped_starter_kits_path = os.path.join(os.getcwd(), 'zipped_starter_kits')
    for item in os.listdir(zipped_starter_kits_path):
        if item.endswith('.zip'):
            current_zip = os.path.join(zipped_starter_kits_path, item)
            zip2structure(current_zip)


if __name__ == "__main__":
    main()