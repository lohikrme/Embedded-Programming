/* WriteAddressBook.cpp
 * To compile the proto file
 * $ protoc --cpp_out=. addressbook.proto
 * To build executable
 * $ g++ -std=c++14 -I/usr/local/include -L/usr/local/lib  WriteAddressBook.cpp addressbook.pb.cc -lprotobuf -pthread  -o WriteAddressBook
 * Note! -pthread sets the flags for preprocessor and linker (in gcc)
 * To compile the proto file
 * $ protoc --cpp_out=. addressbook.proto
 *
*/
#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
using namespace std;

// This function fills in a Person message based on user input.
void PromptForAddress(addressbook::Person* person) {
  cout << "Enter person ID number: ";
  int id;
  cin >> id;
  person->set_id(id);
  cin.ignore(256, '\n');

  cout << "Enter name: ";
  getline(cin, *person->mutable_name());

  cout << "Enter street address: ";
  getline(cin, *person->mutable_street());

  cout << "Enter city info: ";
  getline(cin, *person->mutable_city());

  // cout << "Enter email address (blank for none): ";
  // string email;
  // getline(cin, email);
  // if (!email.empty()) {
  //   person->set_email(email);
  // }

  while (true) {
    cout << "Enter an email address (or leave blank to to proceed with phones): ";
    string email;
    getline(cin, email);
    if (email.empty()) {
      break;
    }

    addressbook::Person::EMailAddress* email_address = person->add_emails();
    email_address->set_email(email);

    cout << "Is this a private or work mail? ";
    string type;
    getline(cin, type);
    if (type == "private") {
      email_address->set_type(addressbook::Person::PRIVATE_MAIL);
    } else if (type == "work") {
      email_address->set_type(addressbook::Person::WORK_MAIL);
    } else {
      email_address->set_type(addressbook::Person::WORK_MAIL);
      cout << "Unknown email type.  Using WORK." << endl;
    }
  }

  while (true) {
    cout << "Enter a phone number (or leave blank to finish): ";
    string number;
    getline(cin, number);
    if (number.empty()) {
      break;
    }

    addressbook::Person::PhoneNumber* phone_number = person->add_phones();
    phone_number->set_number(number);

    cout << "Is this a mobile, home, or work phone? ";
    string type;
    getline(cin, type);
    if (type == "mobile") {
      phone_number->set_type(addressbook::Person::MOBILE_PHONE);
    } else if (type == "home") {
      phone_number->set_type(addressbook::Person::HOME_PHONE);
    } else if (type == "work") {
      phone_number->set_type(addressbook::Person::WORK_PHONE);
    } else {
      phone_number->set_type(addressbook::Person::MOBILE_PHONE);
      cout << "Unknown phone type.  Using mobile." << endl;
    }
  }
}

// Main function:  Reads the entire address book from a file,
//   adds one person based on user input, then writes it back out to the same
//   file.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
    return -1;
  }

  addressbook::Contacts address_book;

  {
    // Read the existing address book.
    fstream input(argv[1], ios::in | ios::binary);
    if (!input) {
      cout << argv[1] << ": File not found.  Creating a new file." << endl;
    } else if (!address_book.ParseFromIstream(&input)) {
      cerr << "Failed to parse address book." << endl;
      return -1;
    }
  }

  // Add an address.
  PromptForAddress(address_book.add_people());

  {
    // Write the new address book back to disk.
    fstream output(argv[1], ios::out | ios::trunc | ios::binary);
    if (!address_book.SerializeToOstream(&output)) {
      cerr << "Failed to write address book." << endl;
      return -1;
    }
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}