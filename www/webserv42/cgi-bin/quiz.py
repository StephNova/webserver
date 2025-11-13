#!/usr/bin/env python3

import cgi
import html # For escaping output

def get_quiz_answers():
    """
    Returns a dictionary of correct answers for each quiz question.
    Keys are question names (e.g., "q1", "q2").
    Values are sets (for multiple choice/checkboxes) or single strings (for radio buttons).
    Using sets for multiple choice allows easy comparison regardless of order.
    """
    return {
        # --- HTTP Basics Quiz ---
        "q1": "GET",  # Radio
        "q2": "OK",   # Radio
        "q3": {"Method", "URI", "HTTP_Version"}, # Checkbox
        "q4": "80",    # Radio
        "q5": "Accept",# Radio
        "q6": "True",  # Radio

        # --- Sockets and Networking Quiz ---
        "q1_sockets": "TCP", # Radio (using different key to avoid collision if all quizzes route to same CGI)
        "q2_sockets": "Assigns_address", # Radio
        "q3_sockets": "accept", # Radio
        "q4_sockets": {"socket", "bind", "listen", "accept"}, # Checkbox (order might matter for flow, but set for comparison)
        "q5_sockets": {"SOCK_STREAM", "SOCK_DGRAM", "SOCK_RAW"}, # Checkbox
        "q6_sockets": "Identify_process", # Radio

        # --- Event Handling (kqueue/poll) Quiz ---
        "q1_events": "Handle_many_clients", # Radio
        "q2_events": "Scalability", # Radio
        "q3_events": "No_wait", # Radio
        "q4_events": "Read_readiness", # Radio
        "q5_events": {"No_iterating", "Edge_triggered", "More_efficient"}, # Checkbox
        "q6_events": "False", # Radio

        # --- HTTP Parsing (Request/Response) Quiz ---
        "q1_parsing": "CRLFCRLF", # Radio
        "q2_parsing": "Content-Length", # Radio
        "q3_parsing": "Request_Line", # Radio
        "q4_parsing": {"HTTP_Version", "Status_Code", "Reason_Phrase"}, # Checkbox
        "q5_parsing": "Length_unknown", # Radio
        "q6_parsing": "Colon", # Radio

        # --- Configuration File Parsing Quiz ---
        "q1_config": "Define_behavior", # Radio
        "q2_config": "Hash", # Radio
        "q3_config": "Recursive_Descent", # Radio
        "q4_config": "Curly_braces", # Radio
        "q5_config": "Instructions", # Radio
        "q6_config": "True", # Radio

        # --- CGI (Common Gateway Interface) Quiz ---
        "q1_cgi": "Dynamic_content", # Radio
        "q2_cgi": "Environment_variables", # Radio
        "q3_cgi": "Stdin", # Radio
        "q4_cgi": "HTTP_headers_body", # Radio
        "q5_cgi": "REQUEST_METHOD", # Radio
        "q6_cgi": "False", # Radio

        # --- Error Handling and Logging Quiz ---
        "q1_errors": "User_experience", # Radio
        "q2_errors": {"Client_IP", "Request_Date", "Request_Line", "Status_Code", "Response_Size"}, # Checkbox
        "q3_errors": "Internal_issues", # Radio
        "q4_errors": "404", # Radio
        "q5_errors": "Stability_performance", # Radio
        "q6_errors": "True", # Radio

        # --- Server Architecture Design Quiz ---
        "q1_arch": "Resource_management", # Radio
        "q2_arch": "Event_driven", # Radio
        "q3_arch": "Background_process", # Radio
        "q4_arch": {"Pipes", "Shared_Memory", "Sockets", "Environment_variables"}, # Checkbox
        "q5_arch": "Pending_connections", # Radio
        "q6_arch": "True", # Radio

        # --- Virtual Hosts Quiz ---
        "q1_vhosts": "Multiple_domains", # Radio
        "q2_vhosts": "Host_header", # Radio
        "q3_vhosts": "root", # Radio
        "q4_vhosts": "Match_host", # Radio
        "q5_vhosts": "Default_server", # Radio
        "q6_vhosts": "False", # Radio

        # --- File Upload and Download Quiz ---
        "q1_files": "multipart/form-data", # Radio
        "q2_files": "Boundary_string", # Radio
        "q3_files": "Content-Disposition", # Radio
        "q4_files": "POST", # Radio
        "q5_files": "application/octet-stream", # Radio
        "q6_files": "True", # Radio
    }

def main():
    form = cgi.FieldStorage()
    correct_answers = get_quiz_answers()
    user_score = 0
    total_questions = 0
    results_html = "<h2>Your Quiz Results:</h2><ul>"

    # Determine which quiz was submitted based on the first question name
    # This is a simple heuristic; a hidden input in the form with `name="quiz_id"` would be better.
    quiz_prefix = ""
    if "q1" in form: # Check for the generic q1 key, implying HTTP Basics
        quiz_prefix = ""
    elif "q1_sockets" in form:
        quiz_prefix = "_sockets"
    elif "q1_events" in form:
        quiz_prefix = "_events"
    elif "q1_parsing" in form:
        quiz_prefix = "_parsing"
    elif "q1_config" in form:
        quiz_prefix = "_config"
    elif "q1_cgi" in form:
        quiz_prefix = "_cgi"
    # elif "q1_errors" in form:
    #     quiz_prefix = "_errors"
    # elif "q1_arch" in form:
    #     quiz_prefix = "_arch"
    # elif "q1_vhosts" in form:
    #     quiz_prefix = "_vhosts"
    elif "q1_files" in form:
        quiz_prefix = "_files"


    # Iterate through expected questions for the identified quiz
    for i in range(1, 7): # Assuming up to 6 questions per quiz
        q_name = f"q{i}{quiz_prefix}"
        if q_name not in correct_answers:
            continue # Skip if this question isn't part of the submitted quiz

        total_questions += 1
        correct_ans = correct_answers[q_name]
        user_ans = form.getlist(q_name) # Use getlist for checkboxes, will return list for radio too (length 1)

        is_correct = False
        if isinstance(correct_ans, set):
            # For multiple choice (checkboxes)
            user_ans_set = set(user_ans)
            if user_ans_set == correct_ans:
                is_correct = True
        else:
            # For single choice (radio buttons)
            if len(user_ans) == 1 and user_ans[0] == correct_ans:
                is_correct = True

        status = "Correct!" if is_correct else "Incorrect."
        user_score += 1 if is_correct else 0
        results_html += f"<li>Question {i}: {status} Your answer(s): {html.escape(', '.join(user_ans))}. Correct answer(s): {html.escape(str(correct_ans))}</li>"

    results_html += "</ul>"
    results_html += f"<p>You scored {user_score} out of {total_questions}!</p>"
    results_html += "<p><a href='/quizzes/'>Back to quizzes</a></p>"

    # --- CGI Output ---
    print("Content-type: text/html\n") # HTTP header, followed by a blank line
    print(f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Quiz Results</title>
        <style>
            body {{ font-family: Arial, sans-serif; line-height: 1.6; margin: 20px; background-color: #f4f4f4; color: #333; }}
            main {{ max-width: 800px; margin: 20px auto; background: #fff; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }}
            h1 {{ color: #0056b3; text-align: center; margin-bottom: 30px; }}
            h2 {{ color: #007bff; margin-top: 25px; }}
            ul {{ list-style-type: none; padding: 0; }}
            li {{ margin-bottom: 10px; padding: 8px; border-bottom: 1px solid #eee; }}
            li:last-child {{ border-bottom: none; }}
            a {{ color: #007bff; text-decoration: none; }}
            a:hover {{ text-decoration: underline; }}
        </style>
    </head>
    <body>
        <main>
            <h1>Quiz Results</h1>
            {results_html}
        </main>
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()