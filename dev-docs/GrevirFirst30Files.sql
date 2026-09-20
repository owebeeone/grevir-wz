-- First 30 distinct planned destination files in each proposed repository.
-- Alphabetical by destination path; contributing source files stay together.
-- A repository with no mapped C++ destination appears with a NULL destination.
SELECT repository, total_destination_files, file_number, destination, sources
FROM first_30_files_per_repository
ORDER BY repository, file_number;
