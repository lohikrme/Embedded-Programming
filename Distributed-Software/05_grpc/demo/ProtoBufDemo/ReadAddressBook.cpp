/* ReadAddressBook.cpp
 * To compile the proto file
 * $ protoc --cpp_out=. addressbook.proto
 * To build executable
 * $ g++ -std=c++14 -I/usr/local/include -L/usr/local/lib  ReadAddressBook.cpp addressbook.pb.cc -lprotobuf -pthread  -o ReadAddressBook
 * Note! -pthread sets the flags for preprocessor and linker (in gcc)
*/

#include <iostream>
#include <fstream>
#include <string>
#include "addressbook.pb.h"
using namespace std;

// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const addressbook::Contacts& address_book) {
  for (int i = 0; i < address_book.people_size(); i++) {
    const addressbook::Person& person = address_book.people(i);

    cout << "Person ID: " << person.id() << endl;
    cout << "  Name: " << person.name() << endl;
    cout << "  Street address: " << person.street() << endl;
    cout << "  City: " << person.city() << endl;

    // if (person.has_email()) {
    //   cout << "  E-mail address: " << person.email() << endl;
    // }
    for (int j = 0; j < person.emails_size(); j++) {
      const addressbook::Person::EMailAddress& email = person.emails(j);

      switch (email.type()) {
        case addressbook::Person::PRIVATE_MAIL:
          cout << "  Private email: ";
          break;
        case addressbook::Person::WORK_MAIL:
          cout << "  Work email: ";
          break;
      }
      cout << email.email() << endl;
    }

    for (int j = 0; j < person.phones_size(); j++) {
      const addressbook::Person::PhoneNumber& phone_number = person.phones(j);

      switch (phone_number.type()) {
        case addressbook::Person::MOBILE_PHONE:
          cout << "  Mobile phone #: ";
          break;
        case addressbook::Person::HOME_PHONE:
          cout << "  Home phone #: ";
          break;
        case addressbook::Person::WORK_PHONE:
          cout << "  Work phone #: ";
          break;
      }
      cout << phone_number.number() << endl;
    }
  }
}

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
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
    if (!address_book.ParseFromIstream(&input)) {
      cerr << "Failed to parse address book." << endl;
      return -1;
    }
  }

  ListPeople(address_book);

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
